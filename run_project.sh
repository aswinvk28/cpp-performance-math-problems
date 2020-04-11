COMMAND="./app \
--intervals 4 \
--iterations 8 \
--condition_factor 2 \
--multiplier 1"

REBUILD=$1
REPO=$2

echo -e "Building docker image\r"
IMAGE_EXISTS=$(sudo docker inspect cpp_math_problems --format={{.Id}})
if [ -z "$IMAGE_EXISTS" ] || [ "$REBUILD" == "rebuild" ]; then
    sudo docker --build_arg REPOSITORY_LINK="$REPO" build -t cpp_math_problems .
fi

echo -e "Building and running app..\r"
CONTAINER_EXISTS=$(sudo docker ps | grep cpp_math_problems)
if [ -z "$CONTAINER_EXISTS" ]; then
    sudo docker run -d -v "$PWD":/repo -it cpp_math_problems
fi
EMPTY_STR=""
CONTAINER_ID=$(sudo docker ps -q)
echo -e "$CONTAINER_ID\r"

sudo docker exec "$CONTAINER_ID" bash -c "mkdir -p /opt/navier-stokes/build && \
    cd /opt/navier-stokes/build && \
    rm -rf * *.* && cmake -DAUTODIFF=/opt/autodiff .. && make all && \
    echo -e '\r\n${COMMAND}\r\n' && \
    $COMMAND"

