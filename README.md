# Docker_server_client
## Usage Instructions

### Local Compilation

#### Server Side:
1. Open terminal.
2. Navigate to the directory containing `server.cpp`.
3. Compile the server code:
   ```bash
   g++ -o server server.cpp
   ```
4. Run the server:
   ```bash
   ./server
   ```

#### Client Side:
1. Open terminal.
2. Navigate to the directory containing `client.cpp`.
3. Compile the client code:
   ```bash
   g++ -o client client.cpp
   ```
4. Run the client with the server's IP address:
   ```bash
   ./client <ip_address>
   ```

### Docker Usage

1. Pull the server and client images from Docker Hub:
   ```bash
   docker pull mohanprasath21/myserver:latest
   docker pull mohanprasath21/myclient:latest
   ```

2. **Server Side:**
   ```bash
   docker run -it --name server-instance mohanprasath21/myserver
   ```

3. **Client Side:**
   ```bash
   docker run -it --name client-instance mohanprasath21/myclient ./client 172.17.0.2
   ```

### Multiple Clients

To run multiple client instances, change the container name for each client:
```bash
docker run -it --name client-instance1 mohanprasath21/myclient ./client 172.17.0.2
```

## Notes
- Ensure that the server is running and accessible before starting the client.
