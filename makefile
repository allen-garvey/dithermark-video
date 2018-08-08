SOURCE_FILES != find ./src -type f -name '*.cpp'
OUTPUT_DIR=bin
OUTPUT=$(OUTPUT_DIR)/dithermark

#for dithering photos
OUTPUT_PHOTO=$(OUTPUT_DIR)/dithermark-photo

all: $(OUTPUT)

install:
	mkdir $(OUTPUT_DIR)

$(OUTPUT): $(SOURCE_FILES)
	g++ -o $(OUTPUT) src/main.cpp -lopencv_imgcodecs -lopencv_core -lopencv_imgproc -lopencv_videoio

$(OUTPUT_PHOTO): $(SOURCE_FILES)
	g++ -o $(OUTPUT_PHOTO) src/picture.cpp -lopencv_imgcodecs -lopencv_core -lopencv_imgproc
