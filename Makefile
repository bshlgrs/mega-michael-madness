all:
	cd quantitative_model && $(MAKE)

deploy:
	$(MAKE) && mv quantitative_model/a.out quantitative_model/run-backend

serve:
	$(MAKE) && bundle exec ruby server.rb 8080

test:
	$(MAKE) && bundle exec ruby server.rb 8787 a.out dev-mode
