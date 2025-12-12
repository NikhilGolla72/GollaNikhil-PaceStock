# Deployment Guide

## GitHub Repository
✅ **Already deployed!** Your code is at:
https://github.com/NikhilGolla72/GollaNikhil-PaceStock

## Vercel Deployment

### Option 1: Deploy via Vercel Dashboard (Recommended)

1. Go to [vercel.com](https://vercel.com) and sign in with GitHub
2. Click "New Project"
3. Import your repository: `NikhilGolla72/GollaNikhil-PaceStock`
4. Vercel will auto-detect the settings:
   - Framework Preset: Other
   - Root Directory: `./`
   - Build Command: (leave empty - static site)
   - Output Directory: `public`
5. Click "Deploy"

### Option 2: Deploy via Vercel CLI

```bash
# Install Vercel CLI
npm i -g vercel

# Login
vercel login

# Deploy
vercel

# Deploy to production
vercel --prod
```

## What Gets Deployed

- **Static Landing Page**: `public/index.html` - Project showcase page
- **API Endpoint**: `api/hello.js` - Simple API endpoint
- **Documentation**: README.md and other docs

## Note About C++ Backend

The C++ backtesting engine cannot run on Vercel (serverless platform). Options:

1. **Keep CLI tool**: Users download and build locally
2. **Add Web API**: Create a Node.js/Python API that calls the C++ binary (requires server)
3. **WASM**: Compile C++ to WebAssembly for browser execution (advanced)

## Current Setup

The repository includes:
- ✅ Static landing page (hostable on Vercel)
- ✅ GitHub Actions CI/CD workflow
- ✅ Complete C++ source code
- ✅ Documentation

Your Vercel site will show the project landing page with links to GitHub and download instructions.

