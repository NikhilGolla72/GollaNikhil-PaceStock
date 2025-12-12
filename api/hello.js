// Simple API endpoint for Vercel
export default function handler(req, res) {
  res.status(200).json({
    name: 'FluxBack',
    description: 'Regime-Aware C++ Backtesting Engine',
    version: '1.0.0',
    endpoints: {
      info: '/api/hello',
      github: 'https://github.com/NikhilGolla72/GollaNikhil-PaceStock'
    }
  });
}

