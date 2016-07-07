require 'sinatra'
require "sinatra/json"
# require_relative 'spreadsheet_reader/spreadsheet_reader'
$TIME_BETWEEN_DOCKER_RESTARTS = 30

set :server, 'thin'
set :bind, '0.0.0.0'
set :port, ARGV[0].to_i
set :show_exceptions, false

get '/causepri-app/' do
  File.open("public/index.html").read
end

post '/eval' do
  inputs = params["inputs"]

  input_to_program = inputs.map do |name, value|
    if value["type"] == "scalar"
      "#{name},#{value["value"]},#{value["value"]}"
    else
      "#{name},#{value["low"]},#{value["high"]}"
    end
  end

  magic_number = Random.rand

  File.write("/tmp/input#{magic_number}", input_to_program.join("\n"))

  executable = "run-backend"
  if ARGV.length > 1
    executable = ARGV[1]
  end

  res = `./quantitative_model/#{executable} /tmp/input#{magic_number}`
  `mv -f /tmp/input#{magic_number} ./quantitative_model/input.txt`
  json(handle_data_lines(res.split("\n")))
end

def get_default_inputs

end

def get_default_outputs

end

def handle_data_lines(lines)
  {}.tap do |data|
    lines.each do |line|
      pieces = line.split(",")
      if pieces.length == 1
        # This line is a warning message or something; skip it
      elsif pieces.length == 2
        data[pieces[0]] = {
          type: "scalar",
          value: pieces[1].to_f
        }
      elsif pieces.length == 3
        data[pieces[0]] = {
          type: "ci",
          low: pieces[1].to_f,
          high: pieces[2].to_f
        }
      else
        raise "Cannot handle line \"#{line}\""
      end
    end
  end
end
