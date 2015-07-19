all: banner storage/sophia.c
	@(cd src; make --no-print-directory)
banner:
	@echo serenity database.
	@echo 
clean:
	@(cd src; make --no-print-directory clean)
	@(cd storage; make --no-print-directory clean)
	@rm -f serenity
storage/sophia.c:
	@(cd storage; make --no-print-directory)
	@echo 
