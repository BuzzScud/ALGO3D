# Build Configuration

This directory contains build configuration files for the ALGO3D project.

## Structure

```
build/
├── config/
│   ├── tailwind.config.js    # Tailwind CSS configuration
│   └── postcss.config.js     # PostCSS configuration
└── README.md                 # This file
```

## Build Commands

### Build Tailwind CSS (Production)
```bash
npm run build-css
```

### Watch Tailwind CSS (Development)
```bash
npm run watch-css
```

## Files

- **tailwind.config.js**: Configures Tailwind CSS content paths and theme settings
- **postcss.config.js**: Configures PostCSS plugins (Tailwind CSS and Autoprefixer)

## Source Files

- **Input**: `assets/css/tailwind-input.css` (source file with Tailwind directives)
- **Output**: `assets/css/tailwind.css` (compiled, minified production CSS)

## Notes

- The build process scans `index.php`, all PHP files, and JavaScript files in `assets/` for Tailwind classes
- The output CSS is minified for production use
- Source maps are not generated (for production builds)


