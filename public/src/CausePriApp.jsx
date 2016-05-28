const CausePriApp = React.createClass({
  getInitialState() {
    return {
      inputs: this.props.initialInputs,
      dataResult: {},
      selectedTab: 0
    }
  },

  handleInputChange(e, inputName, field) {
    var inputs = this.state.inputs;
    inputs[inputName][field] = e.target.value;

    this.setState({
      inputs: inputs
    })
  },

  submit () {
    var that = this;
    $.post("/eval", { inputs: this.state.inputs }, function (e) {
      that.setState({dataResult: e});
    });
  },

  handleTabChange(idx) {
    this.setState({"selectedTab": idx});
  },

  interpretCell(cell) {
    if (cell[0] == "$") {
      return this.output(cell.slice(1));
    } else if (cell[0] == "@") {
      return this.input(cell.slice(1), "value");
    } else {
      return cell;
    }
  },

  firstTr(args) {
    return <tr>{args.map((x, idx) => <th key={idx}>{this.interpretCell(x)}</th>)}</tr>
  },

  tr(args) {
    return <tr>{args.map((x, idx) => <td key={idx}>{this.interpretCell(x)}</td>)}</tr>
  },

  output(name, type) {
    console.log(name);
    return <span>{this.state.dataResult[name] && this.state.dataResult[name][type || "value"] || "unknown"}</span>
  },

  input(name, type, value) {
    return <input
      onChange={(e) => this.handleInputChange(e, name, type)}
      value={this.state.inputs[name] && this.state.inputs[name][type] || value}
      />
  },

  simpleScalarsTable(things) {
    return <Table>
      <tr><th>Variable</th><th>Estimate</th></tr>
      {things.map((row, idx) =>
        <tr key={idx}>
          <td>{row[0]}</td>
          <td>{this.input(row[0], "value", row[1])}</td>
        </tr>)
      }
    </Table>
  },

  simpleDistributionsTable(things) {
    return <Table>
      <tr><th>Variable</th><th>10% CI</th><th>90% CI</th></tr>
      {things.map((row, idx) =>
        <tr key={idx}>
          <td>{row[0]}</td>
          <td>{this.input(row[0], "low", row[1])}</td>
          <td>{this.input(row[0], "high", row[2])}</td>
        </tr>)
      }
    </Table>
  },

  renderMainTab() {
    return <div>
      <h3>Main Tab</h3>

      <Table>
        {this.firstTr(["Intervention", "Mean", "Variance", "posterior", "Notes"])}
        {this.tr(["THL posterior", "$THL estimate p_m", "$THL estimate p_s^2", "$thl_posterior_direct", "this is a note"])}
        {this.tr(["Cage free posterior", "$cage free estimate p_m", "$cage free estimate p_s^2", "$cage_free_posterior_direct", "this is also a note"])}
      </Table>

      <p>This is a paragraph.</p>

      <p>By the way, the far future has value {this.output("EV of far future", "value")}.</p>
    </div>
  },

  renderCageFreeTab() {
    return <div>
      <h3>Cage-free</h3>

      <p>Let's talk about cage free!</p>

      {this.simpleDistributionsTable([
        ["THL years factory farming prevented per $1000",700,13000],
        ["cage-free total expenditures ($M)",2,3],
        ["years until cage-free would have happened anyway",5,10],
        ["millions of cages prevented",100,150],
        ["proportion of change attributable to campaigns",0.7,1],
        ["cage-free years per cage prevented",1,1]
      ])}
    </div>
  },

  renderOtherTab() {
    return <div>
      <h3>Other Tab</h3>

    </div>
  },

  renderGlobalsTab () {
    return <div>
      <h3>Globals</h3>

      <p>Log normal prior parameters</p>

      {this.simpleScalarsTable([
      ["log-normal prior mu",1],
      ["log-normal prior sigma",0.75]
      ])}

      <p>Let's sort out how good we think different beings' lives are, and how much we care about them.</p>

      {this.simpleScalarsTable([
        ["wealthy human well-being", 1],
        ["developing-world human well-being", 0.6],
        ["factory-farmed animal wellbeing", 6],
        ["factory-farmed animal sentience adjustment", 0.3],
        ["cage-free well-being improvement", 1],
        ["wild vertebrate well-being", -2],
        ["wild vertebrate sentience adjustment", 0.2],
        ["insect well-being", 4],
        ["insect sentience adjustment", 0.01],
        ["hedonium well-being", 100],
        ["hedonium brains per human brain", 1000000],
        ["em well-being", 2],
        ["ems per human brain", 1],
        ["paperclip well-being", 0.1],
        ["paperclips per human brain", 1],
        ["dolorium well-being", -100],
        ["dolorium brains per human brain", 1000000]
      ])}
    </div>
  },

  renderFarFutureTab () {
    return <div>
      <h3>Far future</h3>

      <p>How conditionally likely are all these outcomes?</p>

      <p>
        Michael, can you explain how to interpret these?
        I don't know which ones are conditional on which other ones.
      </p>

      {this.simpleScalarsTable([
        ["P(stay on earth)",0.2],
        ["P(we reduce WAS on balance)",0.7],
        ["P(fill universe with biology)",0.4],
        ["P(society doesn't care about animals)",0.8],
        ["P(we have factory farming)",0.2],
        ["P(we spread WAS)",0.4],
        ["P(we make suffering simulations)",0.3],
        ["P(fill universe with computers)",0.4],
        ["P(hedonium)",0.05],
        ["P(ems)",0.3],
        ["P(paperclip)",0.649],
        ["P(dolorium)",0.001]
      ])}

      <p>What is the far future like?</p>


      {this.simpleDistributionsTable([
       ["years of future",1e11,1e12],
       ["accessible stars by computers",1e11,1e14],
       ["usable wattage per star",1e20,1e25],
       ["brains per watt",0.1,0.1],
       ["accessible stars by biology",1e10,1e14],
       ["humans per star",1e10,1e12],
       ["factory farmed animals per star",1e10,1e12],
       ["wild vertebrates per star",1e13,1e16],
       ["insects per star",1e17,1e21],
       ["simulations per insect",1e-3,1]
      ])}

    </div>
  },

  render () {
    var tabs = [this.renderMainTab(), this.renderGlobalsTab(), this.renderFarFutureTab(), this.renderCageFreeTab(), this.renderOtherTab()];

    return <div>
      <div>
          <button
            className="btn pull-right btn-primary"
            onClick={this.submit}>
            Calculate!
          </button>
        <ul className="nav nav-tabs">
          {["Main", "Globals", "Far Future", "Cage Free", "Other"].map((tab, idx) =>
            <li role="presentation" key={idx} className={idx == this.state.selectedTab ? "active" : ""}>
              <a href="#" onClick={() => this.handleTabChange(idx)}>
                {tab}
              </a>
            </li>
          )}
        </ul>

        {tabs[this.state.selectedTab]}
      </div>
    </div>;
  }
});

const Table = React.createClass({
  render () {
    return <table className="table">
      <tbody>
        {this.props.children}
      </tbody>
    </table>
  }
})

$.getJSON("data.json", function(json) {
  ReactDOM.render(<CausePriApp
                    template={json.template}
                    initialInputs={json.inputs}
                    initialOutputs={json.outputs} />, document.getElementById("app-holder"));
});
