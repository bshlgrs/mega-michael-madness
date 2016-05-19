
const CellInput = React.createClass({
  handleInput(e) {
    setVariableValue(parseInt(e.currentTarget.value));
  },
  render: function () {
    return <td style={{"backgroundColor": "yellow"}}>
      <input value={getVariableValue(this.props.name) || this.props.value} onChange={this.handleInput}/>
    </td>;
  }
})

const CellOutput = React.createClass({
  render: function () {
    return <td style={{backgroundColor: "orange"}}>{getVariableValue(this.props.name)}</td>;
  }
})

const CellFormula = React.createClass({
  render: function () {
    return <td style={{backgroundColor: "lightgreen"}}>{this.props.value + " = " + this.props.formula}</td>;
  }
})

var data = {
  "var1": 45,
  "var2": 12,
  "var3": 10
}

var inputs = ["var1"];

function getVariableValue(name) {
  return data[name];
}

function setVariableValue(name, value) {
  data[name] = value;
  changeObservers.forEach((x) => x.notify(name));
}

var changeObservers = [];

const CausePriApp = React.createClass({
  notify: function () {
    this.forceUpdate();
    this.setState({changed: true});
  },

  getInitialState: function () {
    return { "selectedTab": 0, changed: false };
  },

  componentWillMount: function () {
    changeObservers.push(this);
  },

  handleTabChange(idx) {
    this.setState({"selectedTab": idx});
  },

  render: function() {
    var spreadsheet = this.props.spreadsheet;
    return (
      <div>
        <ul className="nav nav-tabs">
          {spreadsheet.map((sheet, idx) =>
            <li role="presentation" key={idx} className={idx == 0 ? "active" : ""}>
              <a href="#" onClick={() => this.handleTabChange(idx)}>
                {sheet.name}
              </a>
            </li>
          )}
        </ul>

        <Sheet sheet={spreadsheet[this.state.selectedTab]} />
        <hr/>
        <button className={"btn btn-lg btn-primary "+(this.state.changed || "disabled")}>Recalculate!</button>
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
          return <SheetTable table={entity.table} title={entity.title} key={idx} />;
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
                  return <CellInput name={cell.name} value={cell.value} key={cellIdx} />;
                } else if (cell.type == "result") {
                  return <CellOutput name={cell.name} value={cell.value} key={cellIdx} />;
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




$.getJSON("spreadsheet.json", function(json) {
  ReactDOM.render(<CausePriApp spreadsheet={json}/>, document.getElementById("app-holder"));
});


