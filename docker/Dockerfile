### Build/test container ###
# Define builder stage
FROM bro-code:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake ..
RUN make
# RUN ctest --output-on_failure
RUN ctest --rerun-failed --output-on-failure

### Deploy container ###
# Define deploy stage
FROM ubuntu:jammy as deploy

# Copy server output binary to "."
COPY --from=builder /usr/src/project/build/bin/server .
COPY --from=builder /usr/src/project/conf/cloudbuild.conf .

# Copy static file content to "."
COPY --from=builder /usr/src/project/data ./data

# Copy boardForm.html to "."
COPY --from=builder /usr/src/project/boardForm.html .

# Expose some port(s)
EXPOSE 80

# Use ENTRYPOINT to specify the binary name
# Update with real server name
ENTRYPOINT ["./server"]

# Use CMD to specify arguments to ENTRYPOINT
# Update with real server args
CMD ["cloudbuild.conf"]
