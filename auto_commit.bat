@echo off
git add .
git commit -m "Auto commit"
git push origin main
echo Auto-commit completed at %date% %time%
pause
