
## 1. copy the file to the amazonlambda docker
```bash
rm -rf /npu-aws-lambda-cpp/

git clone https://github.com/BhanuKiranChaluvadi/NPU.git
docker cp .\npu-aws-lambda-cpp\ trusting_albattani:\

mkdir build && cd build
cd .. && rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=~/install -DCMAKE_PREFIX_PATH=~/install && make
make aws-lambda-package-create_creation
```

## 2. Delete from windows
```bash
Remove-Item -Path .\create_creation.zip -Force

docker cp trusting_albattani:/npu-aws-lambda-cpp/build/src/functions/create_creation/create_creation.zip .
```

## 3. Copy to WSL
```bash
rm -rf build/create_creation.zip
cp /mnt/c/Users/BhanuKiranChaluvadi/github/aws-build/create_creation.zip build/
aws lambda update-function-code     --function-name create_creation     --zip-file fileb://build/create_creation.zip
```