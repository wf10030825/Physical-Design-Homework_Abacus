# MAKEFILE (small fixes)

SRC_DIR := ./src/
OBJ_DIR := ./obj/

################################################################################

SRCS := $(wildcard $(SRC_DIR)*.cpp)
HDRS := $(wildcard $(SRC_DIR)*.h)
OBJS := $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.o,$(SRCS))

# 自動相依（小加：讓 .h 變更會重編）
DEPS := $(OBJS:.o=.d)

################################################################################

TARGET   = HW4_N26141703 # 學號

CXX      = g++
CXXFLAGS = -O3 -Wall -g -std=c++11 -MMD -MP
LIB      =
INCLUDE  = -I$(SRC_DIR)

################################################################################

.PHONY: all clean run
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIB) -o $@

# 編譯規則（自動建立 obj/）
$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

# 需要的資料夾
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# 方便執行：HW4_N26141703 <circuit.aux>
run: $(TARGET)
	./$(TARGET) $(ARGS)

clean:
	rm -f $(TARGET) $(OBJ_DIR)*.o $(OBJ_DIR)*.d core *~

# 讓 .h 相依生效
-include $(DEPS)
