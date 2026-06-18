/**
 * FDM3D-Enclosure 仪表盘服务器
 * 
 * 从串口读取 ESP32 数据，通过 WebSocket 推送到前端
 * v0.2 — 增加: 端口选择, 数据导出, 健康检查, 错误重连
 * 
 * MIT License — FDM3D-Enclosure Project
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

// 历史数据存储 (最多 3600 点 = 5h @ 5s 间隔)
const MAX_HISTORY = 3600;
const history = {
  timestamps: [],
  pm25: [],
  tvoc: [],
  eco2: [],
  temp: [],
  hum: [],
  fan_pwm: []
};

let port = null;
let reconnectTimer = null;
let demoInterval = null;
let isDemoMode = false;

// ============================================================
//  串口管理
// ============================================================

function setupSerial() {
  SerialPort.list().then(ports => {
    // 查找 ESP32 串口
    const espPort = ports.find(p => 
      (p.vendorId && p.vendorId.toLowerCase().includes('10c4')) || // CP210x
      (p.vendorId && p.vendorId.toLowerCase().includes('1a86'))    // CH340
    );

    if (!espPort) {
      console.warn(`[WARN] No ESP32 serial port found. ${ports.length} port(s) available.`);
      if (ports.length > 0) {
        console.log('  Available ports:');
        ports.forEach(p => console.log(`    - ${p.path} (${p.manufacturer || 'unknown'})`));
      }
      console.log('[INFO] Starting demo mode. Use ?port=COMx to force a port.');
      startDemoMode();
      return;
    }

    connectPort(espPort.path);
  }).catch(err => {
    console.error('[ERROR] Serial port list error:', err.message);
    startDemoMode();
  });
}

function connectPort(path) {
  if (port && port.isOpen) return;

  console.log(`[OK] Connecting to ${path}...`);
  port = new SerialPort({
    path: path,
    baudRate: 115200,
    autoOpen: true
  });

  port.on('open', () => {
    console.log(`[OK] Serial port opened: ${path}`);
    stopDemoMode();
    
    const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));
    parser.on('data', handleData);
  });

  port.on('error', (err) => {
    console.error(`[ERROR] Serial: ${err.message}`);
    scheduleReconnect();
  });

  port.on('close', () => {
    console.log('[INFO] Serial port closed');
    scheduleReconnect();
  });
}

function scheduleReconnect() {
  if (reconnectTimer) clearTimeout(reconnectTimer);
  reconnectTimer = setTimeout(() => {
    console.log('[INFO] Attempting serial reconnect...');
    setupSerial();
  }, 10000);  // 10s 后重试
}

// ============================================================
//  数据处理
// ============================================================

function handleData(line) {
  try {
    const data = JSON.parse(line.trim());
    if (data.pm25 !== undefined) {
      broadcastData(data);
    }
  } catch (e) {
    // 非 JSON 行忽略 (启动日志等)
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
  history.fan_pwm.push(data.fan_pwm);

  // 截断
  if (history.timestamps.length > MAX_HISTORY) {
    const excess = history.timestamps.length - MAX_HISTORY;
    history.timestamps.splice(0, excess);
    history.pm25.splice(0, excess);
    history.tvoc.splice(0, excess);
    history.eco2.splice(0, excess);
    history.temp.splice(0, excess);
    history.hum.splice(0, excess);
    history.fan_pwm.splice(0, excess);
  }

  io.emit('sensor-data', {
    current: data,
    history: history
  });
}

// ============================================================
//  演示模式
// ============================================================

function startDemoMode() {
  if (isDemoMode) return;
  isDemoMode = true;
  
  console.log('[Demo] Generating simulated sensor data...');
  demoInterval = setInterval(() => {
    const data = {
      pm25: parseFloat((Math.random() * 80 + 10).toFixed(1)),
      tvoc: Math.floor(Math.random() * 300 + 50),
      eco2: Math.floor(Math.random() * 800 + 400),
      temp: parseFloat((Math.random() * 5 + 25).toFixed(1)),
      hum: parseFloat((Math.random() * 20 + 40).toFixed(1)),
      fan_pwm: Math.floor(Math.random() * 255)
    };
    broadcastData(data);
  }, 5000);
}

function stopDemoMode() {
  if (demoInterval) {
    clearInterval(demoInterval);
    demoInterval = null;
  }
  isDemoMode = false;
  console.log('[OK] Demo mode stopped, real sensor data active');
}

// ============================================================
//  API 路由
// ============================================================

// 健康检查
app.get('/api/health', (req, res) => {
  res.json({
    status: 'ok',
    uptime: process.uptime(),
    mode: isDemoMode ? 'demo' : 'live',
    dataPoints: history.timestamps.length,
    clients: io.engine.clientsCount
  });
});

// 数据导出 CSV
app.get('/api/export/csv', (req, res) => {
  let csv = 'timestamp,pm25,tvoc,eco2,temp,hum,fan_pwm\n';
  for (let i = 0; i < history.timestamps.length; i++) {
    csv += `${history.timestamps[i]},${history.pm25[i]},${history.tvoc[i]},${history.eco2[i]},${history.temp[i]},${history.hum[i]},${history.fan_pwm[i]}\n`;
  }
  res.setHeader('Content-Type', 'text/csv');
  res.setHeader('Content-Disposition', 'attachment; filename=FDM3D-enclosure-data.csv');
  res.send(csv);
});

// 强制指定串口
app.get('/api/port/:name', (req, res) => {
  stopDemoMode();
  if (port && port.isOpen) port.close();
  connectPort(req.params.name);
  res.json({ status: 'connecting', port: req.params.name });
});

// 历史数据 JSON
app.get('/api/history', (req, res) => {
  res.json(history);
});

// ============================================================
//  WebSocket
// ============================================================

io.on('connection', (socket) => {
  console.log(`[WS] Client connected (${io.engine.clientsCount} total)`);

  if (history.timestamps.length > 0) {
    socket.emit('history', history);
  }

  socket.on('disconnect', () => {
    console.log(`[WS] Client disconnected (${io.engine.clientsCount} total)`);
  });
});

// ============================================================
//  启动
// ============================================================

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
  console.log(`\n========================================`);
  console.log(`  FDM3D-Enclosure Dashboard v0.2`);
  console.log(`  http://localhost:${PORT}`);
  console.log(`  http://localhost:${PORT}/api/health`);
  console.log(`========================================\n`);
  
  setTimeout(setupSerial, 1000);
});

// 优雅退出
process.on('SIGINT', () => {
  console.log('\n[INFO] Shutting down...');
  stopDemoMode();
  if (port && port.isOpen) port.close();
  server.close(() => process.exit(0));
});
