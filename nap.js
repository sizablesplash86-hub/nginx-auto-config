const fs = require('fs');
const {exexcSync, exec} = require('child_process');
const os= require('os');

if (process.getuid() !== 0)
{
  console.log("\x1b[31mEnter root first\x1b[0m");
  process.exit(1);
}

//idk