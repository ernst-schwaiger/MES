project(FWMManager)

add_executable(FWMManager 
    src/main.c)

# Link with libraries
target_link_libraries(FWMManager PRIVATE 
    DemoLib
)
