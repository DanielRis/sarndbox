#!/bin/bash
# Set NVIDIA GPU as default for all OpenGL applications
# Run with: sudo ./set-nvidia-default.sh

set -e

if [ "$EUID" -ne 0 ]; then
    echo "Please run with sudo: sudo ./set-nvidia-default.sh"
    exit 1
fi

echo "=== NVIDIA GPU Default Configuration ==="
echo ""
echo "Choose an option:"
echo ""
echo "1) Full NVIDIA mode (prime-select nvidia)"
echo "   - Uses NVIDIA GPU for everything"
echo "   - Better performance, higher power consumption"
echo "   - Requires logout/reboot"
echo ""
echo "2) On-Demand mode with NVIDIA as default for OpenGL"
echo "   - Sets environment variables in /etc/environment"
echo "   - OpenGL apps use NVIDIA, desktop uses Intel"
echo "   - Takes effect on next login"
echo ""
echo "3) Check current configuration"
echo ""
read -p "Enter choice [1-3]: " choice

case $choice in
    1)
        echo ""
        echo "Switching to full NVIDIA mode..."
        prime-select nvidia
        echo ""
        echo "Done! Please logout or reboot for changes to take effect."
        echo "To switch back: sudo prime-select on-demand"
        ;;
    2)
        echo ""
        echo "Setting NVIDIA as default for OpenGL applications..."

        # Backup existing /etc/environment
        cp /etc/environment /etc/environment.backup

        # Add NVIDIA Prime variables if not present
        if ! grep -q "__NV_PRIME_RENDER_OFFLOAD" /etc/environment; then
            echo "" >> /etc/environment
            echo "# NVIDIA GPU as default for OpenGL (added by set-nvidia-default.sh)" >> /etc/environment
            echo "__NV_PRIME_RENDER_OFFLOAD=1" >> /etc/environment
            echo "__GLX_VENDOR_LIBRARY_NAME=nvidia" >> /etc/environment
        else
            echo "NVIDIA environment variables already set in /etc/environment"
        fi

        echo ""
        echo "Done! Please logout and login for changes to take effect."
        echo "Backup saved to /etc/environment.backup"
        echo ""
        echo "To undo: sudo cp /etc/environment.backup /etc/environment"
        ;;
    3)
        echo ""
        echo "=== Current Configuration ==="
        echo ""
        echo "NVIDIA Prime profile:"
        prime-select query 2>/dev/null || echo "  prime-select not available"
        echo ""
        echo "Available GPUs:"
        lspci | grep -E "VGA|3D" || echo "  Could not detect GPUs"
        echo ""
        echo "Current OpenGL renderer:"
        glxinfo 2>/dev/null | grep "OpenGL renderer" || echo "  glxinfo not available (install mesa-utils)"
        echo ""
        echo "NVIDIA environment variables:"
        grep -E "NV_PRIME|GLX_VENDOR" /etc/environment 2>/dev/null || echo "  Not set in /etc/environment"
        ;;
    *)
        echo "Invalid choice"
        exit 1
        ;;
esac
