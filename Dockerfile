# Use a lightweight build image with gcc and libc-dev
FROM gcc:latest AS builder

# Set the working directory inside the container
WORKDIR /usr/src/chat-server

# Copy the source code into the container
COPY ./ChatServer/chat-server.c .

# Compile the application
# We use -static to ensure it runs easily in smaller runtime images
RUN gcc -o chat-server chat-server.c

# Use a smaller runtime image for the final container
FROM debian:bookworm-slim

# Set working directory
WORKDIR /app

# Copy only the compiled binary from the builder stage
COPY --from=builder /usr/src/chat-server/chat-server .

# Expose the port defined in your code (PORT "3490")
EXPOSE 3490

# Run the server
CMD ["./chat-server"]
