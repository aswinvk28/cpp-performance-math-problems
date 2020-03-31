COMMAND="./app \
--intervals 4 \
--iterations 8 \
--condition_factor 2 \
--multiplier 1"

echo -e "Building docker image\r"
IMAGE_EXISTS=$(sudo docker inspect cpp_math_problems --format={{.Id}})
if [ -z "$IMAGE_EXISTS" ]; then
    sudo docker build -t cpp_math_problems .
fi

echo -e "Building and running app..\r"
CONTAINER_EXISTS=$(sudo docker ps | grep cpp_math_problems)
if [ -z "$CONTAINER_EXISTS" ]; then
    sudo docker run -d -v "$PWD":/repo -it cpp_math_problems --name cpp_math_problems
fi
EMPTY_STR=""
CONTAINER_ID=$(sudo docker ps --quiet)

sudo docker exec "$CONTAINER_ID" bash -c "cd /repo/navier-stokes/build && \
        rm -rf * *.* && \
        cmake .. && make all && \
        echo -e '\r\n${COMMAND}\r\n' && \
        $COMMAND"
