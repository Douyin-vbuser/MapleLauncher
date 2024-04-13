const { app, BrowserWindow } = require('electron')
const path = require('node:path')
const axios = require('axios');

function createWindow () {
  const mainWindow = new BrowserWindow({
    width: 900,
    height: 480,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js')
    }
  });

  mainWindow.loadFile('index.html');
  
}

app.on('ready', createWindow);