mcMd_mcMoves_linear_=\
    mcMd/mcMoves/linear/CfbDoubleRebridgeMove.cpp \
    mcMd/mcMoves/linear/CfbEndMove.cpp \
    mcMd/mcMoves/linear/CfbRebridgeMove.cpp \
    mcMd/mcMoves/linear/CfbReptationMove.cpp \
    mcMd/mcMoves/linear/CfbReptateMove.cpp \
    mcMd/mcMoves/linear/EndSwapMove.cpp \
    mcMd/mcMoves/linear/CfbLinear.cpp \
    mcMd/mcMoves/linear/CfbLinearEndMove.cpp 

mcMd_mcMoves_linear_SRCS=\
     $(addprefix $(SRC_DIR)/, $(mcMd_mcMoves_linear_))
mcMd_mcMoves_linear_OBJS=\
     $(addprefix $(BLD_DIR)/, $(mcMd_mcMoves_linear_:.cpp=.o))

