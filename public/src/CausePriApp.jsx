var Modal = ReactBootstrap.Modal;



const CausePriApp = React.createClass({

  // Michael -- You should only have to edit the code between here and the comment where I tell you to stop.

  // To make a new tab, add it to the allTabs() method and then copy one of the tab rendering methods.

  allTabs() {
    return [
      ["Intro", this.renderIntroTab()],
      ["Globals", this.renderGlobalsTab()],
      ["Far Future", this.renderFarFutureTab()],
      ["Cage Free", this.renderCageFreeTab()]
    ]
  },

  renderIntroTab() {
    return <div>
      <h3>Cause prioritization app</h3>

      <p>I will add instruction here at some point.</p>

      <p>See http://mdickens.me/2016/05/17/a_complete_quantitative_model_for_cause_selection/</p>
    </div>
  },

  renderResultsTab() {
    return <div>
      <h3>Main results</h3>

      <Table>
        <tbody>
          {this.firstTr(["Intervention", "Mean", "Variance", "posterior", "Notes"])}
          {this.tr(["THL posterior", "$THL estimate p_m", "$THL estimate p_s^2", "$thl_posterior_direct", "this is a note"])}
          {this.tr(["Cage free posterior", "$cage free estimate p_m", "$cage free estimate p_s^2", "$cage_free_posterior_direct", "this is also a note"])}
        </tbody>
      </Table>

      <p>This is a paragraph.</p>

      <p>By the way, the far future has value {this.output("EV of far future", "value")}.</p>
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
          ["wealthy human well-being", 1, "Centered around 1 by definition"],
          ["developing-world human well-being", 0.6, "Extremely poor people's lives are about half as good as those in the developed world according to world happiness surveys."],
          ["factory-farmed animal wellbeing", 10, "I am willing to trade maybe 10 years normal life vs. 1 year on factory farm."],
          ["factory-farmed animal sentience adjustment", 0.3, "This does not include fish/shellfish."],
          ["cage-free well-being improvement", 1],
          ["wild vertebrate well-being", -2],
          ["wild vertebrate sentience adjustment", 0.2],
          ["insect well-being", 4],
          ["insect sentience adjustment", 0.01],
          ["hedonium well-being", 100, "For the same energy requirements as a human brain."],
          ["hedonium brains per human brain", 1000000],
          ["em well-being", 2, "Basically humans but with less suffering."],
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

          <p>How conditionally likely are all these outcomes? (See notes for conditions)</p>

      {this.simpleScalarsTable([ 
          ["P(stay on earth)",0.2],
          ["P(we reduce WAS on balance)",0.7,"conditional on staying one earth"],
          ["P(fill universe with biology)",0.4],
          ["P(society doesn't care about animals)",0.8,"conditional on filling universe with biology"],
          ["P(we have factory farming)",0.2,"conditional on society doesn't care about animals"],
          ["P(we spread WAS)",0.4,"conditional on society doesn't care about animals"],
          ["P(we make suffering simulations)",0.3,"conditional on society doesn't care about animals"],
          ["P(fill universe with computers)",0.4],
          ["P(hedonium)",0.05,"conditional on filling the universe with computers"],
          ["P(ems)",0.3,"conditional on filling the universe with computers"],
          ["P(paperclip)",0.649,"conditional on filling the universe with computers"],
          ["P(dolorium)",0.001,"conditional on filling the universe with computers"]
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

  //////// MICHAEL, DON'T EDIT BELOW THIS LINE.





















  componentWillMount() {
    setTimeout(() => this.submit(), 1000);
  },

  componentDidMount() {
    if (globalInputs) {
      this.setState({inputs: globalInputs, defaultInputs: JSON.parse(JSON.stringify(globalInputs))});
      this.refs.inputModal.updateTextToString(JSON.stringify(globalInputs));
      globalInputs = null;
    }
  },

  getInitialState() {
    return {
      inputs: {},
      dataResult: {},
      selectedTab: 0,
      showImportModal: false,
      calculating: true
    }
  },

  handleInputChange(e, inputName, field) {
    var inputs = this.state.inputs;
    inputs[inputName][field] = e.target.value;

    this.setState({
      inputs: inputs
    });
  },

  submit () {
    var that = this;
    this.setState({ calculating: true });

    $.post("/eval", { inputs: this.state.inputs }, function (result) {
      if (typeof result == "string") {
        result = JSON.parse(result);
      }

      that.setState({ dataResult: result, calculating: false });
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
    if (this.state.calculating) {
      return <i className="fa fa-spinner fa-spin"></i>
    } else {
      var value = this.state.dataResult[name] && this.state.dataResult[name][type || "value"]
      if (value) {
        return <span>{showFloatNicely(value)}</span>
      } else {
        return <span>unknown</span>
      }
    }
  },

  input(name, type, defaultValue) {
    if (this.state.inputs[name]) {
      if (this.state.inputs[name][type]) {
        var value = showFloatNicely(this.state.inputs[name] && this.state.inputs[name][type]);
      } else {
        var value = ""
      }
    }

    var invalid = isNaN(value) || value == "";

    return <span>
      <input
        className="form-control number-input"
        onChange={(e) => this.handleInputChange(e, name, type)}
        value={typeof value !== "undefined" ? value : defaultValue}
        />
        {invalid && <i className="fa fa-warning fa-spin"></i>}
      </span>
  },

  simpleScalarsTable(things) {
    globalInputs && things.map((row) => globalInputs[row[0]] = {type: "scalar", value: row[1]});

    return <Table>
      <tbody>
        <tr>
          <th></th>
          <th>estimate</th>
          {this.state.displayOriginalInputs && <th>(original)</th>}
          <th>notes</th>
        </tr>
      </tbody>
      {things.map((row, idx) => <ScalarRow
                key={idx}
                row={row}
                displayOriginalInputs={this.state.displayOriginalInputs}
                input={this.input}/>)}
    </Table>
  },

  simpleDistributionsTable(things) {
    globalInputs && things.map((row) => globalInputs[row[0]] = {type: "ci", low: row[1], high: row[2]});

    return <Table>
      <tbody>
        <tr>
          <th></th>
          <th>10% CI</th>
          {this.state.displayOriginalInputs && <th>(original)</th>}
          <th>90% CI</th>
          {this.state.displayOriginalInputs && <th>(original)</th>}
          <th>notes</th>
        </tr>
      </tbody>
      {things.map((row, idx) => <DistributionRow
        key={idx}
        row={row}
        input={this.input}
        displayOriginalInputs={this.state.displayOriginalInputs} />)
      }
    </Table>
  },

  exportInputs(e) {
    e.preventDefault();
  },

  importInputs(e) {
    this.setState({showImportModal: true});
    e.preventDefault();
  },

  closeInputImporter() {
    this.setState({showImportModal: false});
  },

  handleLoadInputs(e, data) {
    e.preventDefault();

    this.setState({ inputs: JSON.parse(data) })
  },

  handleResetInputs(e) {
    e.preventDefault();

    this.setState({ inputs: JSON.parse(JSON.stringify(this.state.defaultInputs)) });
    this.refs.inputModal.updateTextToString(JSON.stringify(this.state.defaultInputs));
  },

  toggleDisplayOriginalInputs(e) {
    this.setState({ displayOriginalInputs: !this.state.displayOriginalInputs });
  },

  render () {
    var tabs = this.allTabs();

    return <div>
      <nav className="navbar navbar-inverse navbar-fixed-top">
        <div className="container-fluid">
          <div className="navbar-header">
            <button type="button" className="navbar-toggle collapsed" data-toggle="collapse" data-target="#navbar" aria-expanded="false" aria-controls="navbar">
              <span className="sr-only">Toggle navigation</span>
              <span className="icon-bar"></span>
              <span className="icon-bar"></span>
              <span className="icon-bar"></span>
            </button>
            <a className="navbar-brand" href="#">Cause prioritization app</a>
          </div>
          <div id="navbar" className="navbar-collapse collapse">
            {false &&<ul className="nav navbar-nav navbar-right">
              <li><a href="#">Menu</a></li>
              <li><a href="#">About</a></li>
              <li><a href="#">Help</a></li>
            </ul>}
            {false && <form className="navbar-form navbar-right">
              <input type="text" className="form-control" placeholder="Search..."/>
            </form>}
          </div>
        </div>
      </nav>

      <div className="container-fluid">
        <div className="row">
          <div className="col-sm-2 col-md-2 sidebar">
            <ul className="nav nav-sidebar">
              {tabs.map((tab, idx) =>
                <li role="presentation" key={idx} className={idx == this.state.selectedTab ? "active" : ""}>
                  <a href="#" onClick={() => this.handleTabChange(idx)}>
                    {tab[0]}
                  </a>
                </li>
              )}
            </ul>

            <hr />
            <ul className="nav">
              <li
                onClick={this.submit}>
                <a className="btn btn-primary">Calculate!</a>
              </li>
              <li
                onClick={this.importInputs}>
                <a className="btn btn-default">Import/export</a>
              </li>
              <li
                onClick={this.handleResetInputs}>
                <a className="btn btn-default">Reset inputs</a>
              </li>
            </ul>

            <div className="checkbox">
              <label>
                <input type="checkbox" onChange={this.toggleDisplayOriginalInputs}/> Display original inputs
              </label>
            </div>
          </div>

          <div className="visible-xs-block">
            <ul className="nav">
              {tabs.map((tab, idx) =>
                <li role="presentation" key={idx} className={idx == this.state.selectedTab ? "active" : ""}>
                  <a href="#" onClick={() => this.handleTabChange(idx)}>
                    {tab[0]}
                  </a>
                </li>
              )}
            </ul>
          </div>

          <div className="col-xs-12 col-sm-10 col-sm-offset-2 col-md-5 col-md-offset-2 main mycontent-left">
            {tabs[this.state.selectedTab][1]}
          </div>
          <div className="hidden-xs hidden-sm results col-md-5">
            {this.renderResultsTab()}
          </div>

          <div className="visible-xs-block visible-sm-block col-xs-12 col-sm-10 col-sm-offset-2">
            {this.renderResultsTab()}
          </div>
        </div>
      </div>

      <InputsImportModal
        show={this.state.showImportModal}
        close={this.closeInputImporter}
        handleLoadInputs={this.handleLoadInputs}
        inputText={JSON.stringify(this.state.inputs)}
        ref="inputModal"/>
    </div>;
  }
});

const Table = React.createClass({
  render () {
    return <div className="table-responsive">
      <table className="table table-striped">
        {this.props.children}
      </table>
    </div>
  }
})


function showFloatNicely(value) {
  if (value && value > 1000000) {
    return parseFloat(value).toExponential()
  }
  return value;
}

const ScalarRow = React.createClass({
  getInitialState () {
    return { showing: false };
  },
  toggleShow(e) {
    this.setState({showing: !this.state.showing});
    e.preventDefault();
  },
  render () {
    var row = this.props.row;
    return <tbody>
      <tr>
        <td>{row[0]}</td>
        <td>{this.props.input(row[0], "value", row[1])}</td>
        {this.props.displayOriginalInputs && <td>{showFloatNicely(row[1])}</td>}
        <td>{row[2] &&
          <a href="#" onClick={this.toggleShow}>
            {this.state.showing ? "hide" : "show"}
          </a>}
        </td>
      </tr>
      {this.state.showing && <tr><td colSpan="4">{row[2]}</td></tr>}
    </tbody>
  }
})

const DistributionRow = React.createClass({
  getInitialState () {
    return { showing: false };
  },
  toggleShow(e) {
    this.setState({showing: !this.state.showing});
    e.preventDefault();
  },
  render () {
    var row = this.props.row;
    return <tbody>
      <tr>
        <td>{row[0]}</td>
        <td>{this.props.input(row[0], "low", row[1])}</td>
        {this.props.displayOriginalInputs && <td>{showFloatNicely(row[1])}</td>}
        <td>{this.props.input(row[0], "high", row[2])}</td>
        {this.props.displayOriginalInputs && <td>{showFloatNicely(row[2])}</td>}
        <td>{row[3] &&
          <a href="#" onClick={this.toggleShow}>
            {this.state.showing ? "hide" : "show"}
          </a>}
        </td>
      </tr>
      {this.state.showing && <tr><td colSpan="6">{row[3]}</td></tr>}
    </tbody>
  }
})

const InputsImportModal = React.createClass({
  componentWillMount () {
    console.log("mounting inputs")
  },
  getInitialState () {
    return { inputText: this.props.inputText };
  },
  updateTextToString(string) {
    this.setState({inputText: string});
  },
  updateText (e) {
    this.setState({inputText: e.target.value});
  },
  render () {
    return <Modal show={this.props.show} onHide={this.props.closeInputImporter}>
      <Modal.Header>
        <Modal.Title>Import inputs</Modal.Title>
      </Modal.Header>
      <Modal.Body>
        <p>Here's all your data. You can copy someone else's data in if you want.</p>
        <textarea rows="10" className="form-control" value={this.state.inputText} onChange={this.updateText}/>
      </Modal.Body>
      <Modal.Footer>
        <a
          onClick={(e) => this.props.handleLoadInputs(e, this.state.inputText)}
          className="btn btn-default pull-left">
          Load into app
        </a>
        <a className="btn btn-primary pull-right" onClick={this.props.close}>Close</a>

      </Modal.Footer>
    </Modal>;
  }
})

var globalInputs = {}

ReactDOM.render(<CausePriApp/>, document.getElementById("app-holder"));
