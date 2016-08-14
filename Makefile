all:
	cd quantitative_model && $(MAKE)

perf:
	cd quantitative_model && $(MAKE) perf

deploy:
	$(MAKE) perf && mv quantitative_model/a.out quantitative_model/run-backend

serve:
	bundle exec ruby server.rb 8080

test:
	bundle exec ruby server.rb 8787 a.out
