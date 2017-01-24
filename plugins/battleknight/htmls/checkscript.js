
var kmUiModule = function(selector, options) {
    this.options = {
        highlight: true
    };
    this.setOptions(options);
    this.initialize(selector);
    //console.log('test.check created! '+JSON.stringify(this.options));
};
kmUiModule.prototype.setOptions = function(options) {
    if(!options) return;
    var props = Object.getOwnPropertyNames(options);
    for(var i = 0; i < props.length; ++i) {
        this.options[props[i]] = options[props[i]];
    }
};
kmUiModule.prototype.initialize = function(selector) {
    var el = document.id(selector);
    if(!el) return;
    this.element = el;
};
kmUiModule.prototype.check = function() {
    if(!this.element) return;
    if(typeof(this.onCheck) === 'function') {
        this.onCheck();
    }
};

var kmUiCheckModule = function(selector, options) {
    kmUiModule.call(this, selector, options);
    if(!this.element) return;
    var el = this.element.getElementsByTagName('input')[0];
    if(!el) return;
    if(el.getAttribute('type') === 'checkbox') {
        el.addEventListener('click', function() {
            account.toggle(this.id, this.checked);
        });
        this.checkbox = el;
    }
};
kmUiCheckModule.prototype = Object.create(kmUiModule.prototype);
kmUiCheckModule.prototype.constructor = kmUiCheckModule;
kmUiCheckModule.prototype.onCheck = function() {
    if(!this.checkbox) return;
    var ist, soll;
    soll = account.isActive('enable'+this.options.name);
    ist = this.checkbox.checked;
    if(soll !== ist) {
        this.checkbox.checked = soll;
    }
};


var kmUiTextModule = function(selector, options) {
    if(!options.formatted) options.formatted = function(str) { return(str); };
    kmUiModule.call(this, selector, options);
    if(!this.element) return;
};
kmUiTextModule.prototype = Object.create(kmUiModule.prototype);
kmUiTextModule.prototype.constructor = kmUiTextModule;
kmUiTextModule.prototype.onCheck = function() {
    if(!this.options.knight_id) return;
    var ist, soll;
    soll = account.profile(this.options.name);
    if(typeof(this.options.formatted) === 'function') {
        soll = this.options.formatted(soll);
    }
    ist = this.element.get('text');
    if(soll !== ist) {
        this.element.set('text', soll);
        if(ist && this.options.highlight) this.element.highlight();
    }
    //console.log('kmUiTextModule.check ' + this.element.id + ', ' + soll);
};







/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////







var kM = kM || function() {};

function checkProfile() {
    myGame.checkModules();
}

// @disable-check M307
var myGame = new kM({ container: 'accountPlugin'
                        , optionsClass: 'kmOptions'
                        , lastDuels: 5
                    });

/********************
 * Turnier Modul
 ********************/
// @disable-check M307
//myGame.gameModules.push(new kmUiCheckModule('kmTurnier',{name:'Turnier'}));

checkProfile();

// @disable-check M307
//var test = new kmUiCheckModule('kmTurnier',{name:'Turnier'});
//test.check();

//console.log('Hello World!');

window.setInterval(checkProfile, 500);

