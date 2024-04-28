const { app, BrowserWindow } = require('electron')
const path = require('node:path')
const request = require('request');
const fs = require('fs');
const axios = require('axios');

function createWindow() {
  const mainWindow = new BrowserWindow({
    width: 900,
    height: 480,
    frame: false
  });

  mainWindow.loadFile('index.html');

}

app.on('ready', createWindow);

app.on('web-contents-created', (e, contents) => {
  contents.on('will-navigate', (event, navigationUrl) => {
    if (navigationUrl.endsWith('exit.html')) {
      app.quit();
    }
  })
});

app.on('ready', () => {
  launcher_folder();
})

app.on('ready', () => {
  const { ipcMain } = require('electron');
  ipcMain.handle('download_environment', () => {
    download_environment();
  });
})

function launcher_folder(){
  const mapleFolderExists = fs.existsSync(path.join(__dirname, 'Maple'));
  if (!mapleFolderExists) {
    fs.mkdirSync(path.join(__dirname, 'Maple'));
    fs.writeFileSync(path.join(__dirname, 'Maple', 'setting.json'), '{}');
    writeSetting('version', 'beta_0.0.1');
    writeSetting('source_server','https://proxy-gh.1l1.icu/https://github.com/Douyin-vbuser/Minecraft-Genshin-Mod/releases/download');
  }
}

function writeSetting(key, value) {
  const filePath = path.join(__dirname, 'Maple', 'setting.json');

  let data = {};
  try {
      data = JSON.parse(fs.readFileSync(filePath, 'utf8'));
  } catch (err) {
      console.error('Error reading setting.json:', err);
  }

  data[key] = value;

  try {
      fs.writeFileSync(filePath, JSON.stringify(data, null, 2));
      console.log('Setting updated successfully.');
  } catch (err) {
      console.error('Error writing setting.json:', err);
  }
}

function readSetting(key) {
  const data = JSON.parse(fs.readFileSync(path.join(__dirname, 'Maple', 'setting.json')));
  return data[key];
}

function download_environment() {
  request(readSetting('source_server')+'/enviroment/default.minecraft.exe')
    .pipe(fs.createWriteStream('.minecraft.exe'))
    .on('finish', function () {
      console.log('Download completed.');
      extract_environment();
    });
}

function extract_environment() {
  const { spawn } = require('child_process');

  const selfExtractingArchivePath = './.minecraft.exe';
  const destinationPath = './';

  const sevenZipSpawn = spawn(selfExtractingArchivePath, [
    '-y', 
    `x`, 
    `-o${destinationPath}` 
  ]);

  sevenZipSpawn.stdout.on('data', (data) => {
    console.log(`stdout: ${data}`);
  });

  sevenZipSpawn.stderr.on('data', (data) => {
    console.error(`stderr: ${data}`);
  });

  sevenZipSpawn.on('close', (code) => {
    console.log(`child process exited with code ${code}`);
  });

}