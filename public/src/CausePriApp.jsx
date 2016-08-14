var Modal = ReactBootstrap.Modal;



const CausePriApp = React.createClass({

  // Michael -- You should only have to edit the code between here and the comment where I tell you to stop.

  // To make a new tab, add it to the allTabs() method and then copy one of the tab rendering methods.

  allTabs() {
    return [
      ["Intro", this.renderIntroTab()],
      ["Globals", this.renderGlobalsTab()],
      ["Basic Interventions", this.renderBasicsTab()],
      ["Far Future", this.renderFarFutureTab()],
      ["Veg Advocacy", this.renderVegTab()],
      ["Cage Free", this.renderCageFreeTab()],
      ["AI Safety", this.renderAISafetyTab()],
      ["Targeted Values Spreading", this.renderTargetedValuesSpreadingTab()],
    ]
  },

  renderIntroTab() {
    return <div>
      <h3>Cause prioritization app</h3>

        <p>
          <a href="http://mdickens.me/2016/04/06/expected_value_estimates_you_can_%28maybe%29_take_literally/">
            Quantitative models offer a superior approach in determining which interventions to support.</a> However,
          naive cost-effectiveness estimates <a
          href="http://blog.givewell.org/2011/08/18/why-we-cant-take-expected-value-estimates-literally-even-when-theyre-unbiased/"> have
          big problems</a>. In particular:
        </p>

        <ol>
          <li>They don’t give stronger consideration to more robust estimates.</li>
          <li>They don’t always account for all relevant factors.</li>
        </ol>

      <p>This is an implementation of <a
        href="http://mdickens.me/2016/05/17/a_complete_quantitative_model_for_cause_selection/">Michael Dickens' attempt</a> to build a quantitative model for cause selection which does not have these limitations.</p>

      <p>The model makes estimates by using expected-value calculations to produce probability distributions of utility values. It then uses these estimates as evidence to update a prior over the effectiveness of different interventions. Treating estimates as evidence updating a prior means that interventions with more robust evidence of effectiveness have better posteriors.</p>

      <p>You can use this app to see the results of the model given various input assumptions. You can see different inputs by clicking on the tabs in the sidebar. After editing them, you can click the "Calculate" button to see how your changed inputs affect the result.</p>

      <p>You can directly use the backend or add new models by <a href="https://github.com/bshlgrs/mega-michael-madness">cloning the GitHub repo</a>.</p>

      <p>This model is a work in progress, and it has some issues. <a href="http://mdickens.me/contact/">Let me know</a> if you see any errors or have suggestions for how to improve it.</p>

      <p>This version was implemented by Michael Dickens and Buck Shlegeris.</p>
    </div>
  },

  renderResultsTab() {
    return <div>
      <h2>Results</h2>

      <h3>Direct effects</h3>

      <p>Measured in terms of <a href="https://en.wikipedia.org/wiki/Quality-adjusted_life_year">QALYs</a> per $1000.</p>

      <Table>
        <tbody>
          {this.firstTr(["Intervention", "Mean", "Sigma", "Posterior"])}
          {this.tr(["GiveDirectly", "$GiveDirectly estimate mean", "$GiveDirectly estimate p_s", "$GiveDirectly posterior"])}
          {this.tr(["Deworm the World", "$DtW estimate mean", "$DtW estimate p_s", "$DtW posterior"])}
          {this.tr(["Veg advocacy", "$veg estimate mean", "$veg estimate p_s", "$veg posterior"])}
          {this.tr(["Cage free", "$cage free estimate mean", "$cage free estimate p_s", "$cage free posterior"])}
        </tbody>
      </Table>

      <h3>Far future effects</h3>

      <Table>
        <tbody>
          {this.firstTr(["Intervention", "Mean", "Sigma", "Posterior"])}
          {this.tr(["AI safety", "$AI safety estimate mean", "$AI safety estimate p_s", "$AI safety posterior"])}
          {this.tr(["Veg advocacy", "$veg ff estimate mean", "$veg ff estimate p_s", "$veg ff posterior"])}
          {this.tr(["Targeted values spreading", "$TVS estimate mean", "$TVS estimate p_s", "$TVS posterior"])}
        </tbody>
      </Table>

      <p><strong>Value of the far future:</strong> {this.output("EV of far future", "value")}</p>

      <p>Sigma (&sigma;) gives the standard deviation of the log base 10 of the distribution. That means &sigma; tells you how the interventions vary in terms of orders of magnitude—so &sigma;=1 means the standard deviation is 1 order of magnitude.</p>
    </div>
  },

  renderGlobalsTab () {
    return <div>
      <h3>Globals</h3>

      <p>Prior distribution weights: how much relative credence should we put in each prior distribution shape?</p>

      {this.simpleScalarsTable([
          ["log-normal weight",1],
          ["Pareto weight",0],
      ])}

      <p><a href="https://en.wikipedia.org/wiki/Log-normal_distribution">Log-normal</a> prior parameters. We write a log-normal distribution as X = 10<sup>m Z + &sigma;</sup> where Z is normally distributed and &mu; = 10<sup>m</sup>.</p>

      {this.simpleScalarsTable([
          ["log-normal prior mu",0.1],
          ["log-normal prior sigma",0.75],
      ])}

      <p><a href="https://en.wikipedia.org/wiki/Pareto_distribution">Pareto</a> prior parameters. We write a Pareto distribution as (&alpha; m<sup>&alpha;</sup>) / (x<sup>&alpha;+1</sup>) where median = m * 2<sup>1/&alpha;</sup>.</p>

      {this.simpleScalarsTable([
          ["Pareto prior median",0.1],
          ["Pareto prior alpha",1.5],
      ])}

      <p>Next establish some basic facts.</p>
      {this.simpleScalarsTable([
        ["interest rate",0.05,"Rate of return on monetary investments."],
      ])}

      <p>Let's sort out how good we think different beings' lives are, and how much they matter. "Well-being" tells us how subjectively valuable a being's experience is, and "sentience adjustment" tells us how sentient a being is relative to humans. So for example, factory farming is really bad, so well-being is below -1, meaning that life on a factory farm is more bad than normal life is good. But chickens are probably less sentient than humans so the sentience adjustment is less than 1.</p>

      {this.simpleScalarsTable([
          ["wealthy human well-being", 1, "Centered around 1 by definition"],
          ["developing-world human well-being", 0.6, "Extremely poor people's lives are about half as good as those in the developed world according to world happiness surveys."],
          ["factory-farmed animal wellbeing", -10, "I would be willing to give up 10 years of normal life to avoid living one year on a factory farm."],
          ["factory-farmed animal sentience adjustment", 0.3, "This does not include fish/shellfish."],
          ["cage-free well-being improvement", 1],
          ["wild vertebrate well-being", -2],
          ["wild vertebrate sentience adjustment", 0.2],
          ["insect well-being", -4],
          ["insect sentience adjustment", 0.01],
          ["hedonium well-being", 100],
          ["hedonium brains per human brain", 1000000],
          ["em well-being", 2, "Basically humans but with less suffering."],
          ["ems per human brain", 1],
          ["paperclip maximizer well-being", 0.1],
          ["paperclip maximizers per human brain", 1],
          ["dolorium well-being", -100],
          ["dolorium brains per human brain", 1000000]
      ])}
    </div>
  },

  renderBasicsTab () {
    return <div>
      <h3>Basic Interventions</h3>

      {this.simpleDistributionsTable([
         ["GiveDirectly",0.9,1.1,"[2]"],
         ["Deworm the World",5,20,"[1][2]. GiveWell rates AMF more highly but I don't endorse the population ethics stance necessary to make AMF look that good (see [3]), so I'm including DtW here as a \"best global poverty charity\"."],
       ])}

      <p>References</p>
      <ol>
        <li>GiveWell, <a href="http://www.givewell.org/international/top-charities/deworm-world-initiative">"Deworm the World Initiative."</a></li>
        <li>GiveWell, <a href="http://www.givewell.org/international/technical/criteria/cost-effectiveness/cost-effectiveness-models">"GiveWell's Cost-Effectiveness Analyses."</a></li>
        <li>Dickens, <a href="http://mdickens.me/2016/05/16/givewell's_charity_recommendations_require_taking_an_unusual_stance_on_population_ethics/">"GiveWell's Charity Recommendations Require Taking a Controversial Stance on Population Ethics."</a></li>
      </ol>

      </div>
  },

  renderFarFutureTab () {
    return <div>
      <h3>Far Future</h3>

          <p>How conditionally likely are all these outcomes? (See notes for conditions, or see the image at [5].)</p>

      {this.simpleScalarsTable([
          ["P(stay on earth)",0.2,"See [4] for explanation."],
          ["P(we reduce WAS on balance)",0.7,"Conditional on staying on earth. WAS = wild animal suffering."],
          ["P(fill universe with biology)",0.4,"See [4], section \"We colonize other planets, a.k.a. Biological Condition\""],
          ["P(society doesn't care about animals)",0.8,"conditional on filling universe with biology"],
          ["P(we have factory farming)",0.2,"Conditional on society doesn't care about animals"],
          ["P(we spread WAS)",0.4,"Conditional on society doesn't care about animals. WAS = wild animal suffering"],
          ["P(we make suffering simulations)",0.3,"Conditional on society doesn't care about animals"],
          ["P(fill universe with computers)",0.4,"See [4], section \"We spread computronium\""],
          ["P(hedonium)",0.05,"Conditional on filling the universe with computers. Hedonium = maximally happy beings experiencing euphoria forever."],
          ["P(ems)",0.3,"Conditional on filling the universe with computers. Ems = computer emulations of human-like brains."],
          ["P(paperclip maximizers)",0.649,"Conditional on filling the universe with computers"],
          ["P(dolorium)",0.001,"Conditional on filling the universe with computers. Dolorium = maximally suffering beings (opposite of hedonium)."],
      ])}

      <p>What is the far future like?</p>

      {this.simpleDistributionsTable([
       ["years of future",1e11,1e12,"[2]"],
       ["accessible stars by computers",1e11,1e14,"[3]"],
       ["usable wattage per star",1e20,1e25,"[3]"],
       ["brains per watt",0.1,0.1,"[3]"],
       ["accessible stars by biology",1e10,1e14,"[3]"],
       ["humans per star",1e10,1e12],
       ["factory farmed animals per star",1e10,1e12],
       ["wild vertebrates per star",1e13,1e16,"[1]; assumes 1-10 planets per star"],
       ["insects per star",1e17,1e21,"[1]"],
       ["simulations per insect",1e-3,1],
      ])}

      <p>References</p>
        <ol>
        <li>Tomasik, <a href="http://reducing-suffering.org/how-many-wild-animals-are-there/">"How Many Wild Animals Are There?"</a></li>
        <li>Wikipedia, <a href="https://en.wikipedia.org/wiki/Timeline_of_the_far_future">"Timeline of the Far Future."</a></li>
        <li>Bradbury, <a href="https://www.gwern.net/docs/1999-bradbury-matrioshkabrains.pdf">"Matrioshka Brains."</a></li>
        <li>Dickens, <a href="http://mdickens.me/2016/04/17/preventing_human_extinction,_now_with_numbers!/">"Preventing Human Extinction, Now With Numbers!"</a></li>
        <li>Dickens, <a href="http://mdickens.me/assets/mermaid/humanity.png">"Far future outcomes tree."</a></li>
        </ol>

    </div>
  },

  renderVegTab() {
    return <div>
      <h3>Veg Advocacy</h3>

      <p>Let's try to figure out if we should advocate for people to care more about farm animals.</p>

      {this.simpleDistributionsTable([
        ["years factory farming prevented per $1000",700,13000,"Estimated by doubling The Humane League's 80% CI from [1]. Excludes shellfish."],
        ["memetically relevant humans",1e9,2e9],
        ["vegetarians per $1000",22,323,"Estimated by doubling the 80% CI for online ads from [2]."],
        ["years spent being vegetarian",5,8,"[2]"],
        ["annual rate at which vegetarians convert new vegetarians",0.005,0.03],
      ])}

      <p>How would animal advocacy affect far-future values?</p>

      {this.simpleDistributionsTable([
        ["factory farming scenarios prevented by changing values",1,1,"Recall that this is conditional on the probabilities given in the far future tab, including P(society doesn't care about animals) and P(we spread factory farming). Without conditioning on those, this value would be a lot lower."],
        ["wild vertebrate suffering prevented by changing values",0.4,0.8,"As a proportion of total suffering (in expectation)."],
        ["insect suffering prevented by changing values",0.2,0.4],
        ["suffering simulations prevented by changing values",0.2,0.4],
        ["hedonium scenarios caused by changing values",0.01,0.1],
        ["dolorium scenarios prevented by changing values",0.001,0.001],
      ])}


     <p>References</p>
     <ol>
        <li>Animal Charity Evaluators, <a href="http://www.animalcharityevaluators.org/research/interventions/impact-calculator/">"Impact Calculator."</a></li>
    <li>Animal Charity Evaluators, <a href="https://docs.google.com/spreadsheets/d/1YSkZDTWacpkmnZMdRsIMLuCOdIILNVmGTIQAomZDxD4">"ACE Leafleting / Online Ads Impact Spreadsheet."</a></li>
    </ol>
    </div>
  },

  renderCageFreeTab() {
    return <div>
      <h3>Cage-Free</h3>

      <p>Let's talk about cage free campaigns!</p>

      {this.simpleDistributionsTable([
        ["cage-free total expenditures ($M)",2,3,"Includes all money spent on cage-free campaigns."],
        ["years until cage-free would have happened anyway",5,10,"[1]"],
        ["millions of cages prevented",100,150,"[1]"],
        ["proportion of change attributable to campaigns",0.7,1],
        ["cage-free years per cage prevented",1,1,"[2]"],
        ["values spreading effect of cage-free year relative to vegetarian-year",0.01,0.1,"I suspect this is fairly low because cage-free has weaker memetic effects than vegetarianism. Lewis Bollard disagrees, see comments [1]."],
      ])}

    <p>References</p>
    <ol>
        <li>Open Philanthropy Project, <a href="http://www.openphilanthropy.org/blog/initial-grants-support-corporate-cage-free-reforms">"Initial Grants to Support Cage-Free Reforms."</a></li>
        <li>United Egg Producers, <a href="http://www.unitedegg.org/GeneralStats/default.cfm">"General US Stats."</a></li>
    </ol>

    </div>
  },

  renderAISafetyTab() {
    return <div>
    <h3>AI Safety</h3>

    <p>For this intervention, we are going to have a mixture over two models. This gives AI Safety a bit of an unfair advantage since the calculations assume that these two models are independent, which unduly increases confidence in them. If you don't like this, you can set one of the model weights to 0.</p>

    <p>Model weights: how much relative credence should we put in each model?</p>

    {this.simpleScalarsTable([
        ["Model 1 weight",0.5],
        ["Model 2 weight",0.5],
    ])}

    <p>General</p>

    {this.simpleDistributionsTable([
        ["cost per AI researcher",70000,150000,"Some uncertainty here about how to account for counterfactuals; presumably AI safety researchers would do something high value otherwise"],
    ])}

    <p>Model 1 (taken from Global Priorities Project [4])</p>

    {this.simpleDistributionsTable([
        ["P(AI-related extinction)",0.03,0.3,"Estimated from [3]. CI for probability represents uncertainty about the estimate."],
        ["size of FAI community when AGI created",200,10000,"[2]"],
        ["AI researcher multiplicative effect",1,3,"If we add one new researcher now, there will be this many new researchers by the time AGI is developed."],
        ["proportion of bad scenarios averted by doubling total research",0.1,0.7],
    ])}

    <p>Model 2</p>

    {this.simpleDistributionsTable([
        ["hours to solve AI safety",1e6,1e10,"Perhaps this should follow Pareto dist? [1]"],
        ["hours per year per AI researcher",2000,2000],
    ])}


    <p>References</p>
    <ol>
        <li>Machine Intelligence Research Institute, <a href="http://intelligence.org/files/PredictingAGI.pdf">"Predicting AGI."</a></li>
        <li>Machine Intelligence Research Institute,  <a href="https://intelligence.org/2014/01/28/how-big-is-ai/">"How Big is the Field of Artificial Intelligence?"</a></li>
        <li>Future of Humanity Institute,  <a href="http://www.fhi.ox.ac.uk/gcr-report.pdf">"Global Catastrophic Risks Survey."</a></li>
        <li>Global Priorities Project, <a href="http://globalprioritiesproject.org/2015/08/quantifyingaisafety/">"How much does work in AI safety help the world?"</a></li>
    </ol>
    </div>
  },

  renderTargetedValuesSpreadingTab() {
     return <div>
     <h3>Targeted Values Spreading</h3>

     How valuable is it to spread good values to AI researchers?

     {this.simpleDistributionsTable([
        ["P(friendly AI gets built)",0.1,0.5,"How should we think of a probability distribution over a probability? There's some sense in which some probability estimates are more precise than others. Maybe think of this as your confidence interval on what your probability estimate would be if you had better information."],
        ["P(AI researchers' values matter)",0.3,0.5,"Given that society doesn't care about animals, what's the probability that improving AI researchers' values will improve an AGI's values?"],
        ["number of AI researchers when AGI created",30000,100000,<span>Different from size of AI safety community; presumably all AI researchers could matter. See <a href="https://intelligence.org/2014/01/28/how-big-is-ai/">here</a>.</span>],
        ["values propagation multiplier",1,5,"If we change one researcher's values today, this many researchers' values will change by the time AGI is developed."],
        ["cost to convince one AI researcher to care about non-human minds ($)",5000,50000],
     ])}

     </div>
  },

  //////// MICHAEL, DON'T EDIT BELOW THIS LINE.





















  componentWillMount() {
    setTimeout(() => this.submit(), 10);
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

    this.refs.inputModal.updateTextToString(JSON.stringify(this.state.inputs));
  },

  submit () {
    var that = this;
    this.setState({ calculating: true });

    $.post("./eval", { inputs: this.state.inputs, defaultInputs: this.state.defaultInputs }, function (result) {
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
      if (value !== undefined) {
        return <span>{showFloatNicely(value)}</span>
      } else {
        return <span>unknown</span>
      }
    }
  },

  input(name, type, defaultValue) {
    if (this.state.inputs[name]) {
      if (this.state.inputs[name][type] !== undefined) {
        var value = showFloatNicely(this.state.inputs[name][type]);
      } else {
        var value = ""
      }
    }

    var invalid = isNaN(value) || value === "";
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
          <th>Estimate</th>
          {this.state.displayOriginalInputs && <th>(original)</th>}
          <th>Notes</th>
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
          <th>Notes</th>
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
            <hr/>
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
  if (value !== undefined && value > 1000000) {
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
