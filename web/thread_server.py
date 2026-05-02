from http.server import HTTPServer, SimpleHTTPRequestHandler


class CustomHandler(SimpleHTTPRequestHandler):
    def end_headers(self):
        # Add custom headers for cross-origin isolation
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        # Call the original end_headers method to finish the response
        super().end_headers()


if __name__ == "__main__":
    PORT = 80
    print(f"Starting HTTP server on http://localhost:{PORT}")

    # Start an HTTP server with the custom handler
    httpd = HTTPServer(('0.0.0.0', PORT), CustomHandler)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down server...")
        httpd.server_close()
