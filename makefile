SOURCE_FILES != find ./src -type f -name '*.cpp'
OUTPUT_DIR=bin
OUTPUT=$(OUTPUT_DIR)/dithermark

all: $(OUTPUT)

install:
	mkdir $(OUTPUT_DIR)

$(OUTPUT): $(SOURCE_FILES)
	g++ -o $(OUTPUT) src/main.cpp -lopencv_imgcodecs -lopencv_core -lopencv_imgproc
