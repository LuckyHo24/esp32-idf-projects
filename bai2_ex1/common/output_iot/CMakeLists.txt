set(pri_req)
idf_component_register(SRCS "output_iot.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_req})