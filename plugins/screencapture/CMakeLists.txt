cmake_minimum_required(VERSION 3.16)
project(screencapture VERSION 1.0.0)

find_package(Albert REQUIRED)
find_package(QCoro6 REQUIRED COMPONENTS Coro)

albert_plugin(
    QT Widgets
    LINK PRIVATE QCoro6::Coro
)
