FROM ubuntu:latest

WORKDIR /falafels

RUN apt update && \
    apt upgrade -y && \
    apt install clang cmake libsimgrid-dev libpugixml-dev -y

COPY CMakeLists.txt /falafels
COPY src/ /falafels/src/
COPY xml/ /falafels/xml/
COPY cmake/ /falafels/cmake
COPY pugixml/ /falafels/pugixml/

RUN (mkdir build && cd build && cmake .. && make)

CMD ["build/main", "xml/platform.xml", "xml/falafels-deployment.xml"]
