const CausePriApp = React.createClass({
  getInitialState() {
    return {
      inputs: this.props.initialInputs,
      dataResult: {}
    }
  },

  handleInputChange(e, inputName, field) {
    var inputs = this.state.inputs;
    inputs[inputName][field] = parseFloat(e.target.value);

    this.setState({
      inputs: inputs
    })
  },

  submit () {
    var that = this;
    $.post("/eval", { inputs: this.state.inputs }, function (e) {
      that.setState({dataResult: JSON.parse(e)});
    });
  },

  renderLine(line) {
    if (line.slice(0,2) == "##") {
      return <h2>{line.slice(2)}</h2>
    } else if (line[0] == "@") {
      var name = line.slice(1).split(",")[0];
      var value = this.state.inputs[name];
      if (value.type == "scalar") {
        return <div>
          <label>
            {name}
          </label>
          <input defaultValue={value.value} onChange={(e) => this.handleInputChange(e, name, "value")}/>
        </div>
      } else if (value.type == "ci") {
        return <div>
          <label>
            {name}
          </label>
          <span>
          <input defaultValue={value.low} onChange={(e) => this.handleInputChange(e, name, "low")}/>
          <input defaultValue={value.high} onChange={(e) => this.handleInputChange(e, name, "high")}/>
          </span>
        </div>
      } else {
        return <div>OH SHIT {name}</div>
      }
    } else if (line[0] == "$") {
      var name = line.slice(1).split(",")[0];
      var value = this.state.dataResult[name];
      if (!value) {
        return <p>{name} = $VALUE</p>
      } else if (value.type == "scalar") {
        return <p>{name} = {value.value}</p>
      } else {
        return <p>{name} = CI({value.low}, {value.high})</p>
      }
    } else {
      return <p>{line}</p>
    }
  },

  render () {
    return <div>
      {this.props.template.split("\n").map((line, idx) => <div key={idx}>{this.renderLine(line)}</div>)}
      <button onClick={this.submit} className="btn btn-large btn-primary">Calculate</button>
    </div>;
  }
});

$.getJSON("data.json", function(json) {
  ReactDOM.render(<CausePriApp
                    template={json.template}
                    initialInputs={json.inputs}
                    initialOutputs={json.outputs} />, document.getElementById("app-holder"));
});
