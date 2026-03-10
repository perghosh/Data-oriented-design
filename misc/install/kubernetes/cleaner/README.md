# Cleaner - Kubernetes Deployment with Nginx

This directory contains the Dockerfile and Kubernetes YAML files to deploy the FileCleaner application with an Nginx web server to a Kubernetes cluster.

## Overview

The `cleaner` application is a powerful command-line tool for analyzing source code files. It counts lines, measures code sections (comments and strings), searches for patterns, and presents detailed statistics.

**Key Features:**
- Count total lines in source files
- Measure code, comments, and strings separately
- Generate character counts
- Process multiple files recursively
- Filter files based on custom criteria
- Search for patterns and list matching lines
- Web interface to view results

**Architecture:**
- Nginx web server provides a web interface on port 80
- Cleaner binary available at `/usr/local/bin/cleaner` (globally accessible)
- Run cleaner on **any directory** inside the pod using `kubectl exec`
- Output files stored in `/app/output` and accessible via the web interface

## Prerequisites

- Docker installed
- Kubernetes cluster running (minikube, kind, or a cloud provider)
- kubectl configured to access your cluster
- The `cleaner` binary must be present in this directory

## Quick Start

### 1. Build the Docker Image

```bash
# Build the Docker image
docker build -t image-cleaner-application:v1 .

# Verify the image
docker images | grep image-cleaner-application
```

### 2. Deploy to Kubernetes

```bash
# Apply the deployment
kubectl apply -f deployment.yaml

# Wait for pods to be ready
kubectl wait --for=condition=ready pod -l app=cleaner-112 --timeout=60s

# Check status
kubectl get pods -l app=cleaner-112
kubectl get svc cleaner-112-service
```

### 3. Access the Web Interface

```bash
# For minikube
minikube service cleaner-112-service

# For other setups, get the external IP
kubectl get svc cleaner-112-service

# Then open in browser: http://<EXTERNAL-IP>:80
```

## Running Cleaner on Any Directory

The cleaner binary is available system-wide at `/usr/local/bin/cleaner`. You can execute it on any directory inside the pod using `kubectl exec`.

### Basic Usage

```bash
# Get help
kubectl exec -it deployment/cleaner-112 -- cleaner help

# Count lines in a directory
kubectl exec deployment/cleaner-112 -- cleaner count --source /usr/local/bin --recursive 1

# Interactive shell
kubectl exec -it deployment/cleaner-112 -- sh
# Inside shell:
cleaner count --source /app/www --recursive 1
```

### Common Commands

| Command | Description |
|---------|-------------|
| `count` | Count lines in files |
| `list` | List lines matching patterns |
| `find` | Search for patterns in files |
| `dir` | List files in directories |
| `history` | View command history |
| `config` | Manage configuration |

### Command Examples

```bash
# Count lines with specific filter
kubectl exec deployment/cleaner-112 -- \
  cleaner count --source /app/www --filter "*.html;*.js" --recursive 1

# Search for patterns
kubectl exec deployment/cleaner-112 -- \
  cleaner find --source /app/www --pattern "function" --recursive 1

# List lines matching regex
kubectl exec deployment/cleaner-112 -- \
  cleaner list --source /app/www --pattern "nginx" --context 2

# Save output to file
kubectl exec deployment/cleaner-112 -- \
  cleaner count --source /app/www --recursive 1 > /app/output/results.txt

# Copy output to local machine
kubectl exec deployment/cleaner-112 -- cat /app/output/results.txt > local_results.txt
```

## Mounting Volumes for Analysis

To analyze code stored in volumes, uncomment and configure the volume mounts in `deployment.yaml`.

### Example 1: Using ConfigMap with Source Code

```bash
# Create a ConfigMap with your code
kubectl create configmap sample-code \
  --from-file=path/to/your/code/main.cpp \
  --from-file=path/to/your/code/helper.h

# Update deployment.yaml to mount the ConfigMap
volumes:
- name: source-code
  configMap:
    name: sample-code

volumeMounts:
- name: source-code
  mountPath: /data/source

# Run cleaner on mounted code
kubectl exec deployment/cleaner-112 -- \
  cleaner count --source /data/source --recursive 3
```

### Example 2: Using Persistent Volume

```bash
# Create a PVC
kubectl apply -f - <<EOF
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: code-storage-pvc
spec:
  accessModes:
  - ReadWriteOnce
  resources:
    requests:
      storage: 10Gi
  storageClassName: standard
EOF

# Update deployment.yaml to mount the PVC
volumeMounts:
- name: code-storage
  mountPath: /data/code

volumes:
- name: code-storage
  persistentVolumeClaim:
    claimName: code-storage-pvc

# Copy code to the volume
kubectl cp ./my-code deployment/cleaner-112:/data/code/

# Run cleaner
kubectl exec deployment/cleaner-112 -- \
  cleaner count --source /data/code --recursive 5
```

### Example 3: Using HostPath (minikube/kind)

```yaml
# Update deployment.yaml
volumeMounts:
- name: host-path
  mountPath: /data/host

volumes:
- name: host-path
  hostPath:
    path: /path/on/host
    type: DirectoryOrCreate
```

## Web Interface Features

The web interface (available at `http://<service-ip>/`) provides:

- **Quick Commands**: Pre-built kubectl commands for common operations
- **Results Display**: View the latest cleaner output
- **Output Files**: Browse generated files via `/output/`
- **Health Check**: `/health` endpoint for monitoring
- **API Endpoint**: `/api/results` to fetch results programmatically

### API Usage

```bash
# Get latest results
curl http://<service-ip>/api/results

# List output files
curl http://<service-ip>/output/

# Download specific output file
curl http://<service-ip>/output/results.txt -o results.txt
```

## Scheduled Analysis

Use CronJobs for automated periodic analysis:

```bash
# Uncomment the CronJob in deployment.yaml
apiVersion: batch/v1
kind: CronJob
metadata:
  name: cleaner-scheduled-analysis
spec:
  schedule: "0 2 * * *"  # Run at 2 AM daily
  jobTemplate:
    spec:
      template:
        spec:
          containers:
          - name: cleaner
            image: image-cleaner-application:v1
            imagePullPolicy: Never
            command: ["/usr/local/bin/cleaner"]
            args: ["count", "--source", "/data/work", "--recursive", "3"]
          restartPolicy: OnFailure
```

## Advanced Configuration

### Custom Nginx Configuration

Edit `nginx.conf` before building the image:

```bash
# Modify nginx.conf
# Rebuild image
docker build -t image-cleaner-application:v1 .
# Re-deploy
kubectl rollout restart deployment/cleaner-112
```

### Resource Limits

Adjust resource requests/limits in `deployment.yaml`:

```yaml
resources:
  requests:
    memory: "256Mi"
    cpu: "200m"
  limits:
    memory: "1Gi"
    cpu: "1000m"
```

### Scaling

```bash
# Scale to 5 replicas
kubectl scale deployment/cleaner-112 --replicas=5

# Auto-scaling (HPA)
kubectl autoscale deployment cleaner-112 --min=3 --max=10 --cpu-percent=80
```

## Troubleshooting

### Pods Not Starting

```bash
# Check pod status
kubectl describe pod <pod-name>

# View logs
kubectl logs <pod-name>

# Check events
kubectl get events --sort-by='.lastTimestamp'
```

### Image Pull Errors

```bash
# Ensure image exists locally
docker images | grep image-cleaner-application

# Verify imagePullPolicy
kubectl get deployment cleaner-112 -o yaml | grep imagePullPolicy
```

### Cleaner Command Not Found

```bash
# Verify binary location
kubectl exec deployment/cleaner-112 -- which cleaner

# Should output: /usr/local/bin/cleaner
```

### Web Interface Not Accessible

```bash
# Check service
kubectl get svc cleaner-112-service

# Check endpoints
kubectl get endpoints cleaner-112-service

# Check pod logs
kubectl logs -l app=cleaner-112
```

### Output Files Not Visible

```bash
# Check output directory
kubectl exec deployment/cleaner-112 -- ls -la /app/output

# Ensure files were created
kubectl exec deployment/cleaner-112 -- cleaner count --source /app/www --recursive 1 > /app/output/test.txt
kubectl exec deployment/cleaner-112 -- cat /app/output/test.txt
```

## Cleaning Up

```bash
# Delete deployment and service
kubectl delete -f deployment.yaml

# Delete specific resources
kubectl delete deployment cleaner-112
kubectl delete service cleaner-112-service

# Delete all resources with the label
kubectl delete all -l app=cleaner-112
```

## Development Tips

### Testing Locally

```bash
# Run container locally
docker run -it --rm -p 8080:80 image-cleaner-application:v1 sh

# Inside container, run cleaner
cleaner count --source /app/www --recursive 1
```

### Debugging

```bash
# Enable shell access
kubectl exec -it <pod-name> -- sh

# Check cleaner version
cleaner version

# Test cleaner on various directories
cleaner count --source /usr/local/bin --recursive 1
cleaner count --source /app/www --recursive 1
cleaner count --source /etc --recursive 1
```

### Monitoring

```bash
# Watch pod status
kubectl get pods -l app=cleaner-112 -w

# View resource usage
kubectl top pods -l app=cleaner-112

# Access nginx logs
kubectl logs -l app=cleaner-112 -c cleaner-container --tail=100 -f
```

## Additional Resources

- [Cleaner Documentation](../../target/TOOLS/FileCleaner/README.MD)
- [Cleaner Commands Guide](../../target/TOOLS/FileCleaner/DOCUMENTATION.MD)
- [Kubernetes Documentation](https://kubernetes.io/docs/)
- [Nginx Documentation](https://nginx.org/en/docs/)

## Support

For issues or questions:
1. Check the troubleshooting section
2. Review pod logs: `kubectl logs -l app=cleaner-112`
3. Check cleaner help: `kubectl exec -it deployment/cleaner-112 -- cleaner help`
4. Consult the FileCleaner documentation in the project