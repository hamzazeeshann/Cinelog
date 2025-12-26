# CINELOG - Quick Start Guide

## ⚡ 5-Minute Setup

### Step 1: Compile the Backend (30 seconds)
```
Double-click: compile.bat
```
Or manually:
```powershell
cd backend
g++ -std=c++17 -o server.exe src/main.cpp -lws2_32
```

### Step 2: Start the Server (5 seconds)
```
Double-click: start_server.bat
```
Or manually:
```powershell
cd backend
./server.exe
```

Wait for: "Server started on port 8080"

### Step 3: Open the Frontend (2 seconds)
```
Open: frontend/index.html in your browser
```

### Step 4: Login
```
Username: admin
Password: admin123
```

## 🎬 That's It!

You now have a fully functional movie rating platform!

---

## 📦 Optional: Add Real Movie Data (5 minutes)

Want 50+ real movies from TMDB?

### Install Python requests library:
```powershell
pip install requests
```

### Run the populator:
```powershell
python populator.py
```

This takes 3-5 minutes due to API rate limiting. Then restart the server to load the new data.

---

## 🆘 Troubleshooting

### "Compilation failed"
- Make sure MinGW is installed and in PATH
- Install MinGW from: https://sourceforge.net/projects/mingw/

### "Port 8080 already in use"
Kill existing process:
```powershell
netstat -ano | findstr :8080
taskkill /F /PID <PID>
```

### "Connection error" in frontend
- Make sure the backend server is running
- Check you see "Server started on port 8080" in the terminal

---

## 📚 For More Details

See the full README.md for:
- Complete API documentation
- Architecture details
- Advanced features
- Troubleshooting guide
