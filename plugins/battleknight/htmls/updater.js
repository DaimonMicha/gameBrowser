
var km_updater = new Class({
    //implements
    Implements: [Options,Events,Chain],
    //options
    options: {
        element: 'countdown',
        start: 10,
        finish: 0,
        progressbars: [],
        startFont: '36px',
        finishFont: '12px',
        onComplete: $empty,
        duration: 1000
    },
    //initialization
    initialize: function(options) {
        //set options
        this.setOptions(options);
        this.options.progressbars = new Array();
        //console.log('km_updater::initialize');
    },
    addProgress: function(topic, module) {
        if(!(this.options.progressbars).contains(topic)) {
            (this.options.progressbars).push(topic);
            if(account.module(topic) !== null) {
                //console.log('addProgress: ' + topic + ' = ' + JSON.stringify(account.module(topic)));
            } else {
                //console.log('addProgress: ' + module + '-' + topic + " = " + account.module(topic));
            }
        }
        //console.log('addProgress: '+ this.options.progressbars.toString());
    },
    update: function() {
        //console.log('km_updater::update');
        (this.options.progressbars).each(function(topic, index) {
            this.updateProgress(topic);
        },this);
    },
    updateProgressText: function(topic, text) {
        document.id('progressbarText'+topic).getElement('div.km_progressText').set('text', 'unknown');
        document.id('progressbarText'+topic).getElement('div.km_progressText').set('text', text);
    },
    updateProgressTime: function(topic, text) {
        document.id('progressbarText'+topic).getElement('span').set('text', text);
    },
    updateProgressMeter: function(topic, width) {
        document.id('timeMeter'+topic).setStyle('width', width + '%');
    },

    updateProgress: function(topic) {
        var eta = parseInt(account.status('timer'+topic));
        var progressbarWidth = 0;
        var text = '';
        if(eta > 0) {
            var duration = parseInt(account.status('timer'+topic+'Duration'));
            var values = countdown(eta);
            text += values.hours + ':' + values.minutes + ':' + values.seconds;
            var dir = 'up';
            if(typeof document.id('timeMeter'+topic).get('km_direction') !== 'undefined') {
                dir = document.id('timeMeter'+topic).get('km_direction');
            }
            //if(dir === null || dir === 'up') progressbarWidth = (100 - ((100 / duration) * eta).toInt());
            if(dir === 'down') progressbarWidth = ((100 / duration) * eta).toInt();
            else  progressbarWidth = (100 - ((100 / duration) * eta).toInt());
            //console.log('timer: ' + topic + ', ' + dir);
            if(typeof document.id('progressbarText'+topic).get('km_timer') !== 'undefined') {
                if(document.id('progressbarText'+topic).get('km_timer') === 'on') {
                    this.updateProgressTime(topic, text);
                }
            }

            this.updateProgressMeter(topic, progressbarWidth);
            //console.log('update: ' + topic);
        }
    }
});

// @disable-check M307
var kmUpdater = new km_updater();

function updater() {
    kmUpdater.update();
}

window.setInterval(updater, 1000);

