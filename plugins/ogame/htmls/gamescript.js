
var kmCheckerModule = function() {
    var options = {name:'kmCheckerModule'};
    function setOptions(options) {
    }
};

var kM = kM || function() {
    this.version = '0.0.1';
    this.options = {
        container: 'accountPlugin',
        optionsClass: 'kmOptions'
    };

    this.setOptions = function(options) {
        Object.keys(options).forEach(function(val, idx){
            this.options[val] = options[val];
        },this);
    };

    for(var key in arguments) {
        var value = arguments[key];

        if(typeof(value) === 'string') {
            this.options.container = value;
            this.optionsUI();
        } else if(typeof(value) === 'object') {
            this.setOptions(value);
            this.optionsUI();
        }
    }
};

kM.prototype.optionsUI = function(value) {
    var p = document.getElementById(this.options.container);
    if(!p) return;
    var inputs = p.getElementsByTagName('input');

    //alert(JSON.stringify(this.options));// + ' = ' + JSON.stringify(value));

    setCheckBoxHandler = function(el) {
        el.addEventListener('click', function() { account.toggle(this.id, this.checked); });
    }

    for(var i = 0; i < inputs.length; i++) {
        var input = inputs[i];
        switch(input.type) {
            case 'checkbox':
                if(input.classList.contains(this.options.optionsClass)) {
                    if(account.isActive(input.id)) input.setAttribute('checked', 'checked');
                    setCheckBoxHandler(input);
                }
                break;
            default:
                break;
        }
    }
};

