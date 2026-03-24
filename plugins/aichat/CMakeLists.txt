cmake_minimum_required(VERSION 3.16)
project(aichat VERSION 1.0.0)

find_package(Albert REQUIRED)
find_package(QCoro6 REQUIRED COMPONENTS Coro Core Network)

albert_plugin(
    QT Widgets Network
    LINK PRIVATE QCoro6::Coro QCoro6::Core QCoro6::Network
)
