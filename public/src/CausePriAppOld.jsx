const CausePriApp = React.createClass({

  getInitialState() {
    return {
      selectedTab: 0,
      changed: false,
      outputs: this.props.initialOutputs,
      inputs: this.props.initialInputs
    };
  },

  getOutputValue(name) {
    return this.state.outputs[name] || "$VALUE";
  },

  getInputValue(name) {
    return this.state.inputs[name];
  },

  setInputValue(name, value) {
    var inputs = this.state.inputs;
    inputs[name] = value
    this.setState({inputs: inputs, changed: true});
  },

  handleTabChange(idx) {
    this.setState({"selectedTab": idx});
  },

  handleEvaluationRequest () {
    var that = this;
    $.post("./eval", {inputs: this.state.inputs}, function(data) {
      var result = JSON.parse(data);
      that.setState({ outputs: result.outputs, changed: false })
    });
  },

  render() {
    var spreadsheet = this.props.spreadsheet;
    return (
      <div>
        <ul className="nav nav-tabs">
          {spreadsheet.map((sheet, idx) =>
            <li role="presentation" key={idx} className={idx == this.state.selectedTab ? "active" : ""}>
              <a href="#" onClick={() => this.handleTabChange(idx)}>
                {sheet.name}
              </a>
            </li>
          )}
          <button
            className={"btn pull-right btn-primary "+(this.state.changed || "disabled")}
            onClick={this.handleEvaluationRequest}>
            Recalculate!
          </button>
        </ul>

        <Sheet
          sheet={spreadsheet[this.state.selectedTab]}
          setInputValue={this.setInputValue}
          getInputValue={this.getInputValue}
          getOutputValue={this.getOutputValue}
          />
        <hr/>

      </div>
    );
  }
});

const Sheet = React.createClass({
  render: function () {
    return <div>
      {this.props.sheet.entities.map((entity, idx) => {
        if (entity.type == "h3") {
          return <h3 key={idx}>{entity.value}</h3>;
        } else if (entity.type == "p") {
          return <p key={idx}>{entity.value}</p>;
        } else if (entity.type == "table") {
          return <SheetTable
                  table={entity.table}
                  title={entity.title}
                  key={idx}
                  setInputValue={this.props.setInputValue}
                  getInputValue={this.props.getInputValue}
                  getOutputValue={this.props.getOutputValue} />;
        }
      })}
    </div>
  }
});

const SheetTable = React.createClass({
  render: function () {
    return <div>
      {this.props.title && <h4>{this.props.title}</h4>}

      <table className="table table-bordered table-condensed">
        <tbody>
          {this.props.table.map((row, rowIdx) =>
            <tr key={rowIdx}>
              {row.map((cell, cellIdx) => {
                if (!cell) {
                  return <td key={cellIdx}/>;
                } else if (cell.type == "input") {
                  return <CellInput
                            name={cell.name}
                            key={cellIdx}
                            setInputValue={this.props.setInputValue}
                            getInputValue={this.props.getInputValue} />;
                } else if (cell.type == "result") {
                  return <CellOutput
                            name={cell.name}
                            key={cellIdx}
                            getOutputValue={this.props.getOutputValue} />;
                } else if (cell.type == "th") {
                  return <th key={cellIdx}>{cell.value}</th>;
                } else if (cell.type == "td") {
                  return <td key={cellIdx}>{cell.value}</td>;
                } else {
                  return <CellFormula key={cellIdx} formula={cell.formula} value={cell.value}/>;
                }
              })}
            </tr>
          )}
        </tbody>
      </table>
    </div>;
  }
});

const CellInput = React.createClass({
  getInitialState () {
    return { value: this.props.getInputValue(this.props.name) }
  },
  handleInput(e) {
    this.setState({ value: e.currentTarget.value });
  },
  onBlur() {
    this.props.setInputValue(this.props.name, parseFloat(this.state.value));
  },
  render: function () {
    return <td style={{"backgroundColor": "yellow"}}>
      <input value={this.state.value} onChange={this.handleInput} onBlur={this.onBlur}/>
    </td>;
  }
})

const CellOutput = React.createClass({
  render: function () {
    return <td style={{backgroundColor: "orange"}}>{this.props.getOutputValue(this.props.name).toFixed(2)}</td>;
  }
})

const CellFormula = React.createClass({
  render: function () {
    return <td style={{backgroundColor: "lightgreen"}}>{this.props.value + " = " + this.props.formula}</td>;
  }
})

$.getJSON("data.json", function(json) {
  ReactDOM.render(<CausePriApp
                    spreadsheet={json.spreadsheet}
                    initialInputs={json.inputs}
                    initialOutputs={json.outputs} />, document.getElementById("app-holder"));
});


