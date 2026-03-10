

## Samples

### Hello Kubernetes, simple nginx deployment

**Step 1: Create a deployment and expose it to your local machine **
```powershell
# Create a deployment running 2 instances of Nginx
kubectl create deployment hello-k8s --image=nginx

# Scale it to 3 replicas just to see the orchestration in action
kubectl scale deployment hello-k8s --replicas=3

# Expose the deployment to your local machine on port 8091 (this will create a service that forwards traffic to the deployment)
kubectl expose deployment hello-k8s --type=LoadBalancer --port=8091 --target-port=80 --name=hello-service

# Open page with nginx welcome message in your browser using localhost:8091
```

**Step 2: Check the status of the deployment and pods, and scale it**

*List active pods using* `kubectl get pods`  

If you want to stop the deployment, you can run: `kubectl scale deployment hello-k8s --replicas=0`.  
This will stop all the pods but keep the deployment definition, so you can easily scale it back up later.

Now if you run `kubectl get pods` again, you will see that all the pods are stopped.

To scale it up again, just run: `kubectl scale deployment hello-k8s --replicas=3` and it will start 3 pods again.

**Delete the deployment and service**

This deletes the deployment and all the pods it created, and also deletes the service that was exposing it.
```
kubectl delete deployment hello-k8s
kubectl delete service hello-service # deletes 'hello-service'
```

or with a one-liner: `kubectl delete deployment,service hello-k8s hello-service`

Note that you will get errors that kubernetes now cant find the deployment and service, but you can ignore those since we just deleted them.

*Tip: list deployments that you can stop* 
`kubectl get deployments | Select-Object -Skip 1 | ForEach-Object { $name = $_.Split(' ')[0]; kubectl scale deployment $name --replicas=0 }`


### Same as above but with a bit more flexibility

```powershell
# --- CONFIGURATION ---
$sPodName = "sample-01"
$sServiceName = "$sPodName-service"
$iExternalPort = 8091
$iInternalPort = 80
$iReplicaCount = 3

# --- EXECUTION ---
# Create the deployment
kubectl create deployment $sPodName --image=nginx

# Scale the deployment
kubectl scale deployment $sPodName --replicas=$iReplicaCount

# Expose the deployment
kubectl expose deployment $sPodName --type=LoadBalancer --port=$iExternalPort --target-port=$iInternalPort --name=$sServiceName

# --- VERIFICATION ---
Write-Host "Success! Access your app at http://localhost:$iExternalPort" -ForegroundColor Cyan
kubectl get pod,svc -l app=$sPodName
```

### # Check resource usage

kubectl describe pod <pod-name>
kubectl top pod <pod-name> # if metrics-server is installed

### Work with pods as remote servers

kubectl exec <pod name> -- ls # list files in the pod's filesystem

As you can se, to execute a command in the pod, you can use `kubectl exec` followed by the pod name and the command you want to run. This allows you to interact with the pod as if you were logged into it, which is useful for debugging and managing your applications running in Kubernetes.  
But to simplify it, you can also get into interactive bash shell inside the pod, which can be more convenient for exploring and running multiple commands.
`kubectl exec -it <pod name> -- bash`  
To exit type `exit` or press `Ctrl + D`.  

Copy file into pod: `kubectl cp <local file path> <pod name>:<remote file path>`


## Minikube

### Start a local Kubernetes cluster with Minikube
```
minikube start
```

### Reload image if needed
```
minikube image load cleaner-app:v1 --overwrite
```


## Git

### Download repos from github and run them in kubernetes

```
# Download the main branch as a tarball
curl -L https://github.com/user/repo/tarball/main -o repo.tar.gz

# Extract it into your work directory
tar -xzf repo.tar.gz -C /data/work --strip-components=1
```