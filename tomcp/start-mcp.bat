@echo off
REM Start UMICP MCP Bridge on Windows via WSL
wsl -d Ubuntu-24.04 -- bash -l -c "cd /mnt/f/Node/hivellm/umicp/tomcp && node dist/index.js"


