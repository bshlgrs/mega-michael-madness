require 'sinatra'
require "sinatra/json"
require "sinatra/cookies"
require "mongo"

$collection = Mongo::Client.new('mongodb://127.0.0.1:27017/causepri-app-logs')[:logs]

set :server, 'thin'
set :bind, '0.0.0.0'
set :port, ARGV[0].to_i
set :show_exceptions, false

get '/' do
  File.open("public/index.html").read
end

post '/eval' do
  cookies[:uid] ||= Random.rand(1_000_000_000_000)
  inputs = params["inputs"]
  default_inputs = params["defaultInputs"]
  consider_rfmf = params["considerRfmf"]

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

  res = `./quantitative_model/#{executable} /tmp/input#{magic_number} #{consider_rfmf}`
  `mv -f /tmp/input#{magic_number} ./quantitative_model/input.txt`

  response = handle_data_lines(res.split("\n"))

  user_modified_inputs = inputs.select { |x| inputs[x] != default_inputs[x] }
  unmodified_default_inputs = inputs.select { |x| inputs[x] == default_inputs[x] }

  $collection.insert_one({
    user_modified_inputs: user_modified_inputs,
    unmodified_default_inputs: unmodified_default_inputs,
    outputs: response,
    visitor_id: cookies[:uid],
    ip: request.ip,
    created_at: DateTime.now
  })

  json(response)
end

get "/analytics" do
  count = $db.execute("SELECT COUNT(*) FROM evaluation_requests")[0][0]
  "You have had #{count} evaluation attempts!"
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
