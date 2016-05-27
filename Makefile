all:
	cd quantitative_model && $(MAKE)

debug:
	cd quantitative_model && $(MAKE) debug

serve:
	bundle exec ruby server.rb 8080
