
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
        var eta;
        var modul = '', category = topic;
        var result = topic.match(/.[A-Z][a-z]/g);
        if(result) {
            for(idx = 0; idx < result.length; ++idx) {
                var text = new String(result[idx]);
                var pos = topic.indexOf(''+text.charAt(1), 1);
                modul = topic.substring(0, pos);
                category = topic.substring(pos);
            }
            eta = parseInt(account.state(modul, 'timer' + category));

            //console.log('result (' + topic + '), ' + modul + ', ' + category + ', ' + text.length + ', ' + result + ': ' + eta );
        }

        if(account.module(modul) !== null) {
            //console.log('updateProgress: ' + topic + ' = ' + JSON.stringify(account.module(modul)));
        }

        if(modul === '') modul = topic;

        var progressbarWidth = 0;
        text = '';
        if(eta > 0) {
            //var duration = parseInt(account.status('timer'+topic+'Duration'));
            var duration = parseInt(account.state(modul, 'timer'+category+'Duration'));
            //console.log('duration: ' + duration);
            var values = countdown(eta);
            text += values.hours + ':' + values.minutes + ':' + values.seconds;
            var dir = 'up';
/*
            if(typeof document.id('timeMeter'+topic).get('km_direction') !== 'undefined') {
                dir = document.id('timeMeter'+topic).get('km_direction');
            }
*/
            var timeMeter = document.id('timeMeter'+topic);
            if(typeof timeMeter !== 'undefined') {
                if(timeMeter.hasClass("timeMeterDown")) dir = "down";
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

