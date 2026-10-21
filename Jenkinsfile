
pipeline {
    agent none

    environment {
        AWS_REGION = 'ap-south-1'
        ECR_REGISTRY = '013644998183.dkr.ecr.ap-south-1.amazonaws.com'
        ECR_REPOSITORY = 'matair'
        IMAGE_TAG = "build-${BUILD_NUMBER}"
    }

    stages {
        stage('Checkout') {
            agent { label 'built-in' }
            steps {
                checkout scm
                sh 'git log -1 --oneline'
            }
        }

        stage('Build and Test') {
            agent { label 'built-in' }
            steps {
                sh '''
                    set -eu
                    docker build \
                      -f dockerfile \
                      -t matair:test-${BUILD_NUMBER} .
                    docker image inspect matair:test-${BUILD_NUMBER}
                '''
            }
        }

        stage('Tag and Push to ECR') {
            agent { label 'built-in' }
            steps {
                sh '''
                    set -eu
                    aws sts get-caller-identity
                    aws ecr get-login-password --region "$AWS_REGION" |
                      docker login --username AWS --password-stdin "$ECR_REGISTRY"

                    docker tag matair:test-${BUILD_NUMBER} \
                      "$ECR_REGISTRY/$ECR_REPOSITORY:$IMAGE_TAG"

                    docker push "$ECR_REGISTRY/$ECR_REPOSITORY:$IMAGE_TAG"
                '''
            }
        }

        stage('Deploy to Minikube') {
            agent { label 'matair-windows' }
            steps {
                bat '''
                    powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "$env:IMAGE_TAG='%IMAGE_TAG%'; & 'D:\\airlinesys\\deploy-matair.ps1'; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }"
                '''
            }
        }
    }

    post {
        success {
            echo 'MatAir CI/CD pipeline completed successfully.'
        }
        failure {
            echo 'Pipeline failed. Check the stage logs above.'
        }
    }
}
