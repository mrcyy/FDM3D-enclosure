/**
 * FDM3D-Enclosure 仪表盘服务器
 * 
 * 从串口读取 ESP32 数据，通过 WebSocket 推送到前端
 */

const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const { SerialPort, ReadlineParser } = require('serialport');
const path = require('path');

const app = express();
const server = http.createServer(app);
const io = new Server(server);

// 静态文件
app.use(express.static(path.join(__dirname, 'public')));

// 历史数据存储 (最多 1000 点)
const history = {
  timestamps: [],
  pm25: [],
  tvoc: [],
  eco2: [],
  temp: [],
  hum: []
};
const MAX_HISTORY = 1000;

// 串口配置
let port;

function setupSerial() {
  SerialPort.list().then(ports => {
    // 自动查找 ESP32 串口 (常见 VID/PID)
    const espPort = ports.find(p => 
      p.vendorId && p.vendorId.toLowerCase().includes('10c4') || // CP210x
      p.vendorId && p.vendorId.toLowerCase().includes('1a86')    // CH340
    );

    if (!espPort) {
      console.warn('[WARN] No ESP32 serial port found. Running in demo mode.');
      startDemoMode();
      return;
    }

    port = new SerialPort({
      path: espPort.path,
      baudRate: 115200
    });

    const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));
    parser.on('data', handleData);
    console.log(`[OK] Connected to ${espPort.path}`);
  }).catch(err => {
    console.error('[ERROR] Serial port error:', err.message);
    startDemoMode();
  });
}

function handleData(line) {
  try {
    const data = JSON.parse(line.trim());
    if (data.pm25 !== undefined) {
      broadcastData(data);
    }
  } catch (e) {
    // 非 JSON 行忽略 (如固件启动日志)
  }
}

function broadcastData(data) {
  const now = new Date().toISOString();
  
  // 更新历史
  history.timestamps.push(now);
  history.pm25.push(data.pm25);
  history.tvoc.push(data.tvoc);
  history.eco2.push(data.eco2);
  history.temp.push(data.temp);
  history.hum.push(data.hum);

  // 截断历史
  if (history.timestamps.length > MAX_HISTORY) {
    history.timestamps.shift();
    history.pm25.shift();
    history.tvoc.shift();
    history.eco2.shift();
    history.temp.shift();
    history.hum.shift();
  }

  // 广播到所有 WebSocket 客户端
  io.emit('sensor-data', {
    current: data,
    history: history
  });
}

// 演示模式: 生成模拟数据
function startDemoMode() {
  console.log('[Demo] Generating simulated sensor data...');
  setInterval(() => {
    const data = {
      pm25: Math.random() * 80 + 10,
      tvoc: Math.floor(Math.random() * 300 + 50),
      eco2: Math.floor(Math.random() * 800 + 400),
      temp: Math.random() * 5 + 25,
      hum: Math.random() * 20 + 40,
      fan_pwm: Math.floor(Math.random() * 255)
    };
    broadcastData(data);
  }, 5000);
}

// WebSocket 连接
io.on('connection', (socket) => {
  console.log(`[WS] Client connected`);

  // 发送历史数据
  if (history.timestamps.length > 0) {
    socket.emit('history', history);
  }

  socket.on('disconnect', () => {
    console.log(`[WS] Client disconnected`);
  });
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
  console.log(`\n========================================`);
  console.log(`  FDM3D-Enclosure Dashboard`);
  console.log(`  http://localhost:${PORT}`);
  console.log(`========================================\n`);
  
  // 延迟启动串口 (等服务器就绪)
  setTimeout(setupSerial, 1000);
});
