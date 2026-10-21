@'
$ErrorActionPreference = "Stop"

Write-Host "Checking Minikube..."
kubectl get nodes

Write-Host "Refreshing ECR credentials..."
$password = aws ecr get-login-password --region ap-south-1
if ($LASTEXITCODE -ne 0 -or !$password) {
    throw "Could not obtain ECR credentials."
}

kubectl create secret docker-registry ecr-secret `
  --docker-server=013644998183.dkr.ecr.ap-south-1.amazonaws.com `
  --docker-username=AWS `
  --docker-password=$password `
  --dry-run=client -o yaml | kubectl apply -f -

if ($LASTEXITCODE -ne 0) {
    throw "Failed to refresh the Kubernetes ECR secret."
}

if (!$env:IMAGE_TAG) {
    throw "IMAGE_TAG was not supplied by Jenkins."
}

$image = "013644998183.dkr.ecr.ap-south-1.amazonaws.com/matair:$env:IMAGE_TAG"

Write-Host "Deploying image $image"
kubectl set image deployment/matair matair=$image
if ($LASTEXITCODE -ne 0) { throw "Failed to update the deployment." }

kubectl rollout status deployment/matair --timeout=180s
if ($LASTEXITCODE -ne 0) { throw "Deployment rollout failed." }

kubectl get pods -l app=matair
Write-Host "MatAir deployment completed successfully."
'@ | Set-Content -Path .\deploy-matair.ps1