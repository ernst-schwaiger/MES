project(FWMgmnt)

add_executable(FWMgmnt 
    src/main.c)

# Link with libraries
target_link_libraries(FWMgmnt PRIVATE 
    DemoLib
)
