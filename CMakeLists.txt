cmake_minimum_required(VERSION 3.10)
project(dms)

# 하위 프로젝트 추가
add_subdirectory(dms_server)
add_subdirectory(dms_client)