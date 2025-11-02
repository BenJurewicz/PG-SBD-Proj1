default:
	@make -C ./build

debug:
	@make -C ./build-debug

clean:
	rm -f out/*
