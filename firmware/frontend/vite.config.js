import { defineConfig } from 'vite';

export default defineConfig({
  build: {
    outDir: '../data', // Output build to a 'dist' folder outside 'frontend'
    emptyOutDir: true, // Clear the output directory before building
  },
  test: {
    environment: 'jsdom',
  },
  server: {
    host: true,
    watch: {
      usePolling: true,
    },
    port: 3000, // Frontend dev server port
    proxy: {
      '/log': 'http://localhost:8080',
      '/config': 'http://localhost:8080',
    }
  },
});
