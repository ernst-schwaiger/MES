project(KeyMgmnt)

add_executable(KeyMgmnt 
    src/main.c)

# Link with libraries
target_link_libraries(KeyMgmnt PRIVATE 
    DemoLib
)
