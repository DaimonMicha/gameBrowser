
window.onerror = null;

var targetAreas = {
    "Attack": {
        "1": ["r-S", "rechte Schulter"],
        "2": ["-K-", "Kopf"],
        "3": ["l-S", "linke Schulter"],
        "4": ["r-A", "rechter Arm"],
        "5": ["-B-", "Brust"],
        "6": ["l-A", "linker Arm"],
        "7": ["r-B", "rechtes Bein"],
        "8": ["-Z-", "Zentrum"],
        "9": ["l-B", "linkes Bein"]
    },
    "Defend": {
        "1": ["r-O", "rechter Oberkörper"],
        "2": ["l-O", "linker Oberkörper"],
        "3": ["-Z-", "Zentrum"],
        "4": ["r-S", "rechte Seite"],
        "5": ["l-S", "linke Seite"]
    }
}

function randInt(low, high) {
    return(Math.floor(Math.random() * (high - low + 1)) + low);
}

var kmTextoutModule = function() {
    this.options = {
        template: 'kmZonesResult',
        selector: 'fightResults',
        position: 'left',
        knight_id: 0,
        onCheckAll: function() {
            //console.log(this.name+': Check for ' + this.knight_id);
            this.onCheck();
        },
        onCheck: function() {
            if(!this.element) return;
            var hits, defs;
            hits = account.player(this.knight_id,'hitZones');
            defs = account.player(this.knight_id,'defendZones');
            if(hits !== this.hitZones || defs !== this.defZones) {
                var table = this.element.getFirst('table');
                if(!table) {
                    this.element.set('text', soll);
                } else {
                    if(hits.length > 0 && defs.length > 0) {
                        var hZones = JSON.parse(hits);
                        var dZones = JSON.parse(defs);
                        var rows = table.getFirst('tbody').getElements('tr');
                        for(var c = 0;c < rows.length; ++c) {
                            var cols = rows[c].getElements('td');
                            var ist = cols[1].get('text');
                            if(ist !== targetAreas.Attack[hZones[c]][1]) {
                                cols[1].set('text', targetAreas.Attack[hZones[c]][1]);
                                if(ist) cols[1].highlight();
                            }
                            ist = cols[3].get('text');
                            if(ist !== targetAreas.Defend[dZones[c]][1]) {
                                cols[3].set('text', targetAreas.Defend[dZones[c]][1]);
                                if(ist) cols[3].highlight();
                            }
                        }
                        //console.log(this.name+' hitZones\t:' + hits + ':' + defs);
                    }
                }
                this.hitZones = hits;
                this.defZones = defs;
                //console.log(this.name+': Zones-Check for ' + this.knight_id);
            }
        }
    };
    this.check = function(module) {
        if(typeof(this.options.onCheck) === 'function') {
            this.options.onCheck();
            //console.log(JSON.stringify(typeof(this))+', '+JSON.stringify('onCheck'+module));
        }
    };
    this.initialize = function(options) {
        var props = Object.getOwnPropertyNames(options);
        for(var i = 0; i < props.length; ++i) {
            this.options[props[i]] = options[props[i]];
        }
        var el = document.id(this.options.template);
        if(!el) return;
        var resultDiv = document.getElement('div.'+this.options.selector);
        if(!resultDiv) return;

        el = el.clone();
        el.set('id','km'+this.options.name+'Textout');
        el.setStyle('float',this.options.position);
        if(this.options.position === 'left') {
            el.setStyles({'left':'0px','text-align':'right'});
        } else if(this.options.position === 'right') {
            el.setStyles({'right':'0px','text-align':'left'});
        }

        el.inject(resultDiv);
        el.removeClass('nodisplay');
        this.options.element = el;

    };
    this.initialize(arguments['0']);
    //console.log('CREATE kmTextoutModule('+this.options.name+'): '+JSON.stringify(arguments['0']));
}


var kmParserModule = function(myName, options) {
    this.module_name = myName || 'kmParserModule';
    this.options = {
        container: 'kmParser',
        optionsClass: 'kmOptions',
        lastDuels: 5
    };
    this.setOptions = function(options) {
        Object.keys(options).forEach(function(val, idx){
            this.options[val] = options[val];
        },this);
    };
    this.initialize(options);
    //console.log('#-> kmParserModule: ' + JSON.stringify(this));
}
kmParserModule.prototype.initialize = function(options) {
    for(var key in options) {
        var value = options[key];

        if(typeof(value) === 'number' || typeof(value) === 'string') {
            this.options[key] = value;
        } else if(typeof(value) === 'object') {
            this.setOptions(value);
        } else {
            //console.log('typeof(value) =' + typeof(value) + ', ' + key + ' = ' + value);
        }
    }
    var eid = this.options.container;
    var element = document.getElementById(eid);
    if(!element) {
        eid = 'km' + this.module_name;
        element = document.getElementById(eid);
    }
    if(!element) return;
    this.options.container = eid;
    var cname = this.options.optionsClass;

    element.addEventListener('click', function(e) {
        var e = e || window.event;
        var target = e.target || e.srcElement;

        if(target.nodeName === 'INPUT' && target.hasClass(cname)) {
            account.toggle(target.id, target.checked);
        }
/*
        console.log('ModulListener(click) ' + target.nodeName
                    + ': ' + JSON.stringify(target.type)
                    + ', id=' + target.id);
*/
    });

}; // kmParserModule.prototype.initialize
kmParserModule.prototype.check = function(module) {
    if(typeof(this.onCheck) === 'function') this.onCheck(module);
/*
    console.log('ModulListener(check) ' + this.module_name
                + ': ' + JSON.stringify(this)
                + ', id=' + this.options.container);
*/
}; // kmParserModule.prototype.check

var kM = kM || function() {
    this.version = '0.0.1';
    this.module_name = 'klickMeister';
    this.options = {
        container: 'accountPlugin',
        optionsClass: 'kmOptions',
        lastDuels: 5
    };
    this.data = {};
    this.modules = [];

    this.setOptions = function(options) {
        Object.keys(options).forEach(function(val, idx){
            this.options[val] = options[val];
        },this);
    };

    for(var key in arguments) {
        var value = arguments[key];

        if(typeof(value) === 'number' || typeof(value) === 'string') {
            this.options[key] = value;
        } else if(typeof(value) === 'object') {
            this.setOptions(value);
        }
    }

    this.gameModules = [];


    /************************************************************
     * Account Modul
     ************************************************************/
    // @disable-check M307
    this.gameModules.push(new kmUiCheckModule('kmAccount',{name:'Account'}));

    /********************
     * Knight's Name
     ********************/
    // @disable-check M307
    this.gameModules.push(new kmUiTextModule('km_knightName',{name:'knight_name',knight_id:account.profile('knight_id')}));

    /********************
     * Treasury Cooldown
     ********************/
    // @disable-check M307
    this.gameModules.push(new kmUiTextModule('kmTreasuryTimer',{formatted:function(str){
        str = parseInt(account.status(this.name));
        if(isNaN(str)) return(this.textBevore + 'bereit.');
        var values = countdown(str);
        return(this.textBevore + values.hours + ':' + values.minutes + ':' + values.seconds);
    },name:'timerTreasury',knight_id:account.profile('knight_id'),textBevore:'Schatzkammer: ',highlight:false}));

    /********************
     * Knight's Location
     ********************/
    // @disable-check M307
    this.gameModules.push(new kmUiTextModule('km_location',{formatted:function(str){
        if(account.profile(this.name)) str = km_locations[account.profile(this.name)].title;
        return(str);
    },name:'location',knight_id:account.profile('knight_id')}));



    /************************************************************
     * Missions Modul
     ************************************************************/
    // @disable-check M307
    //this.gameModules.push(new kmUiCheckModule('kmMissions',{name:'Missions'}));

    /********************
     * Mission Points
     ********************/
    // @disable-check M307
    //this.gameModules.push(new kmUiTextModule('km_missionPoints',{name:'missionPoints',knight_id:account.profile('knight_id')}));



    // @disable-check M307
/*
    module = new kmParserModule('GM',{'container':'kmGM','optionsClass':this.options.optionsClass});
    module.onCheck = function() {
        var ist, soll;
        soll = account.isActive('enable'+this.module_name);
        ist = document.id('enable'+this.module_name).checked;
        if(soll !== ist) {
            document.id('enable'+this.module_name).checked = soll;
        }
        // GroupMission Points
        ist = document.id('km_gmPoints').get('text');
        soll = account.profile('gmPoints');
        if(soll !== ist) {
            document.id('km_gmPoints').set('text', soll)
            if(ist) document.id('km_gmPoints').highlight();
        }
    };
    this.gameModules.push(module);
*/
    // @disable-check M307
/*    module = new kmParserModule('ClanWar',{'container':'kmClanWar','optionsClass':this.options.optionsClass});
    module.onCheck = function() {
        var ist, soll;
        soll = account.isActive('enable'+this.module_name);
        ist = document.id('enable'+this.module_name).checked;
        if(soll !== ist) {
            document.id('enable'+this.module_name).checked = soll;
        }
    };
    this.gameModules.push(module); */
    // @disable-check M307
    module = new kmParserModule('Duels',{'container':'kmDuels','optionsClass':this.options.optionsClass,'lastDuels':this.options.lastDuels});
    module.onCheck = function() {
        var ist, soll;
        soll = account.isActive('enable'+this.module_name);
        ist = document.id('enable'+this.module_name).checked;
        if(soll !== ist) {
            document.id('enable'+this.module_name).checked = soll;
        }
        buildReportRow = function(id, report) {
            //alert(id + ': ' + JSON.stringify(report));
            var newRow = new Element('tr', {'id': 'report_'+id, 'class': 'reportToolTip'});
            var newEl = new Element('td');
            var icon = new Element('span', {'class': 'duelIcon'}); //iconWins
            var stats;
            var opponentId;

            if(parseInt(account.profile('knight_id')) === parseInt(report.aggressor_id)) {
                stats = report.fight_stats.aggressor;
                opponentId = parseInt(report.defender_id);
            } else {
                stats = report.fight_stats.defender;
                opponentId = parseInt(report.aggressor_id);
                newRow.setStyle('background-color', 'rgba(165, 42, 42, 0.45)');
            }
            var tipText = account.profile('knight_name');
            if(stats.fightResult === 'won') {
                icon.addClass('iconWins');
                tipText += ' erbeutet '+stats.silver+' Silber von ';
            } else {
                icon.addClass('iconLosses');
                tipText += ' verliert '+Math.abs(parseInt(stats.silver))+' Silber an ';
            }
            icon.inject(newEl);
            tipText += account.player(opponentId,'knight_name')+'.';
            //tipText += '<br />'+this.buildZonesTooltip(opponentId);
            var fTime = report.fight_time;//.split(' ')[1];
            var pos = fTime.lastIndexOf(':')
            fTime = fTime.substr(0,pos);
            newRow.setAttribute('onClick', 'g_notify.alert("Duell ('+fTime+')", "'+tipText+'", {duration: 5000});');
            newRow.store('tip:title', 'Duell ('+fTime+')');
            newRow.store('tip:text', tipText);
            newEl.inject(newRow);
            newEl = new Element('td', {'align': 'right'});
            var knightLink = new Element('a', {'class': 'specialKM', 'rel': '#modal_kmKnight', 'onClick': 'window.km_currentKnight='+opponentId+';'});
            knightLink.set('text', account.player(opponentId,'knight_name'));

            knightLink.inject(newEl);
            newEl.inject(newRow);
            return(newRow);
        }

        //console.log(JSON.stringify(this));
        var temp = account.reports(parseInt(this.options.lastDuels),'duel');

        if(!temp) return;
        var duelsTableBody = document.id('duelTable').getLast();

        var istRows = duelsTableBody.getElementsByTagName('tr');

        var duels = JSON.decode(temp);
        var keys = Object.keys(duels);
        //keys.reverse();
        Array.each(keys,function(entry) {
            var report = duels[entry];
            if(!document.id('report_'+entry)) {
                var row = buildReportRow(entry, report);
                if(row) {
                    if(istRows.length >= this.options.lastDuels) {
                        // letzte zeile entfernen
                        duelsTableBody.removeChild(duelsTableBody.getLast());
                    }
                    //oben einfügen
                    row.inject(duelsTableBody,'top');
                }
            }
        },this);
    };
    this.gameModules.push(module);
    // @disable-check M307
/*    module = new kmParserModule('Turnier',{'container':'kmTurnier','optionsClass':this.options.optionsClass});
    module.onCheck = function() {
        var ist, soll;
        soll = account.isActive('enable'+this.module_name);
        ist = document.id('enable'+this.module_name).checked;
        if(soll !== ist) {
            document.id('enable'+this.module_name).checked = soll;
        }
    };
    this.gameModules.push(module); */

    this.checkModules = function() {
        this.gameModules.forEach(function(v){v.check();},this);
    }

    this.parseDocument();
    this.checkModules();

};

kM.prototype.parseDocument = function() {
    var paths = [];
    Array.each(window.location.pathname.split('/'),function(item){
        if(item !== '') paths.push(item);
    });

    if(!paths.length) return;
    var status = {};

    var mainContent = document.id('mainContent');
    status.mainContent = mainContent.classList.toString();
    status.contentTitle = document.id('contentTitle').get('text').trim();

    if(mainContent.hasClass('cooldownDuel')) status.waitReason = 'Duel';
    else if(mainContent.hasClass('cooldownWork')) status.waitReason = 'Work';
    else if(mainContent.hasClass('cooldownTravel')) status.waitReason = 'Travel';
    else if(mainContent.hasClass('cooldownFight')) status.waitReason = 'Mission';

    if(typeof(progressbarEndTime) !== 'undefined') {
        status.waitTime = progressbarEndTime;
        if(typeof(progressbarDuration) !== 'undefined') {
            status.waitDuration = progressbarDuration;
            if(typeof(l_callUrl) !== 'undefined') {
                status.waitCaller = l_callUrl;
            }
        }
    } else if(typeof(l_titleTimerEndTime) !== 'undefined') {
        status.waitTime = l_titleTimerEndTime;
    }


    /*
     * parses the document for acc-players data
    */
    parsePlayer = function(status) {
        status.knight_id = parseInt(document.id('shieldNeutral').href.split('/')[5]);
        status.knight_name = document.id('life').retrieve('tip:title').split(', Stufe')[0];
        status.knight_level = parseInt(document.id('userLevel').get('text').trim());
        status.life = parseInt(document.id('lifeCount').get('text').trim());
        status.maxLife = g_maxHealth;
        status.experience = parseInt(document.id('levelCount').get('text').trim());
        status.silver = parseInt(document.id('silverCount').get('text').trim());
        status.treasury = parseInt(document.id('silver').retrieve('tip:text').split(': ')[1]);
        status.rubies = parseInt(document.id('rubyCount').get('text').trim());
        if(document.id(document.body).hasClass('evil')) status.knight_course = 'evil';
        else status.knight_course = 'good';

        status.module = 'player';
        return(status);
    } // parsePlayer

    /*
     * parses the document of location baseUrl()+"common"
    */
    parseCommon = function(status, paths) {
        var path = paths.shift();
        var ret = {};

        // parses "/common/profile"
        parseProfile = function(kid) {
            var ret = {};

            ret.knight_id = parseInt(kid);
            var container = document.id('profileImage');
            container = container.getElement('h2');
            var temp = container.get('text').trim();
            var pos = temp.indexOf(' ');
            ret.knight_rang = temp.substr(0,pos);
            ret.knight_name = temp.substr(pos+1);
            container = document.id('profileDetails');
            var rows = container.getElements('td');
            rows.each(function(row,index){
                var tmp = row.get('text').trim();
                switch(index) {
                    case 0:
                        ret.knight_level = parseInt(tmp);
                        break;
                    case 1:
                        var link = row.getFirst();
                        if(link) {
                            ret.clan_id = parseInt(link.pathname.split('/')[3]);
                            ret.clan_tag = tmp.substr(1,tmp.length-2);
                        }
                        break;
                    case 2:
                        if(ret.clan_id) ret.clan_rang = tmp;
                        break;
                    case 3:
                        ret.loot_won = parseInt(tmp);
                        break;
                    case 4:
                        ret.loot_lose = parseInt(tmp);
                        break;
                    case 5:
                        ret.turniere = parseInt(tmp);
                        break;
                    case 6:
                        ret.fights_won = parseInt(tmp);
                        break;
                    case 7:
                        ret.fights_balance = parseInt(tmp);
                        break;
                    case 8:
                        ret.fights_lose = parseInt(tmp);
                        break;
                    default:
                        break;
                }
            },this);
            container = document.id('profileAttrib');
            rows = container.getElements('td');
            rows.each(function(row,index){
                var tmp = row.get('text').trim();
                switch(index) {
                    case 0: // Stärke
                        ret.strength = parseInt(tmp);
                        break;
                    case 1: // Geschick
                        ret.dexterity = parseInt(tmp);
                        break;
                    case 2: // Konstitution
                        ret.endurance = parseInt(tmp);
                        break;
                    case 3: // Glück
                        ret.luck = parseInt(tmp);
                        break;
                    case 4: // Waffenkunst
                        ret.weapon = parseInt(tmp);
                        break;
                    case 5: // Verteidigungskunst
                        ret.shield = parseInt(tmp);
                        break;
                    case 6: // Schaden
                        tmp = tmp.split(' - ');
                        ret.damage_min = tmp[0];
                        ret.damage_max = tmp[1];
                        break;
                    case 7: // Rüstung
                        ret.armour = parseInt(tmp);
                        break;
                    default:
                        break;
                }
            },this);

            ret.submodule = 'parseProfile';
            return(ret);
        } // parseProfile

        switch(path) {
            case 'profile':
                ret = parseProfile(paths.shift());
                account.setPlayer(ret);
                break;
            default:
                break;
        }

        return(ret);
    } // parseCommon

    /*
     * parses the document of location baseUrl()+"user"
    */
    parseUser = function(paths) {
        var path = paths.shift();
        var ret = {};

        parseTravel = function() {
            var ret = {};

            return(ret);
        } // parseTravel

        switch(path) {
            case 'travel':
                //break;
            default:
                ret = parseTravel();
                break;
        }

        return(ret);
    } // parseUser

    /*
     * parses the document of location baseUrl()+"world"
    */
    parseWorld = function(paths) {
        var path = paths.shift();
        var ret = {};

        parseLocation = function() {
            var ret = {};

            if(document.id('mainContent').hasClass('location')) {
                ret.location = document.id('mainContent').classList["1"];
                ret.missionPoints = parseInt(document.id('zoneChangeCosts').get('text').trim());
            }

            ret.subModule = 'location';
            return(ret);
        } // parseLocation

        switch(path) {
            case 'travel':
                break;
            case 'location':
            default:
                ret = parseLocation();
                break;
        }

        ret.module = 'world';
        return(ret);
    } // parseWorld

    /*
     * parses the document of location baseUrl()+"groupmission"
    */
    parseGroupmission = function(paths) {
        var path = paths.shift();
        var ret = {};

        parseGroup = function() {
            var ret = {};
            if(!document.id('mainContent').hasClass('tavern')) return(ret);
            if(document.id('selectfoundPlandata')) {
                ret.gmPoints = 120;
            } else {
                var div = document.querySelector('div.innerContent');
                div = document.id(div);
                if(div) {
                    div = div.getFirst().getFirst();
                    ret.gmPoints = parseInt(div.get('text').trim());
                }
            }
            if(typeof(inGroup) === 'boolean') {
                ret.inGroup = inGroup;
                ret.duration = randInt((1000*23),(1000*60*5));
                if(ret.inGroup) {
                    ret.gmPoints = 120;
                    //if(!ret.to) ret.to = setTimeout(this.groupTimer, ret.duration, this);
                }
            }
            ret.subModule = 'group';
            return(ret);
        } // parseGroup

        switch(path) {
            case 'group':
                //break;
            default:
                ret = parseGroup();
                break;
        }

        return(ret);
    } // parseGroupmission

    /*
     * parses the document of location baseUrl()+"market"
    */
    parseMarket = function(paths) {
        var path = paths.shift();
        var ret = {};

        parseTravel = function() {
            var ret = {};

            return(ret);
        } // parseTravel

        switch(path) {
            case 'travel':
            default:
                ret = parseTravel();
                break;
        }

        return(ret);
    } // parseMarket

    /*
     * parses the document of location baseUrl()+"duel"
    */
    parseDuel = function(status,paths) {
        var path = paths.shift();
        var ret = {};

        parseDuel = function(status) {
            status.defender = parseInt(window.location.search.split('enemyID=').getLast());
            status.submodule = 'parseDuel';
            return(status);
        } // parseDuel

        switch(path) {
            case 'duel':
                ret = parseDuel(status);
                break;
            case 'compare':
            default:
                break;
        }

        return(ret);
    } // parseDuel

    /*
     * parses the document of location baseUrl()+"joust"
    */
    parseJoust = function(status, paths) {
        var path = paths.shift();
        var ret = {};

        parseJoust = function(status) {
            var ret = {};

            return(ret);
        } // parseTravel

        switch(path) {
            case 'zones':
            case 'tent':
            default:
                ret = parseJoust(status);
                break;
        }

        return(ret);
    } // parseJoust

    /*
     * parses the document of location baseUrl()+"clan"
    */
    parseClan = function(status, paths) {
        var path = paths.shift();
        var ret = {};

        parseMembers = function() {
            var ret = {};
            ret.members = [];

            var cmTable = document.id('membersTable');
            var cmBody = cmTable.getElement('tbody');
            var cmRows = cmBody.getElements('tr');
            cmRows.each(function(row){
                var cmCols = row.getElements('td');
                var player = {};
                cmCols.each(function(col){
                    if(col.hasClass('memberName')) {
                        var link = col.getFirst();
                        player.knight_id = parseInt(link.pathname.split('/')[3]);
                        var temp = link.get('text');
                        var pos = temp.indexOf(' ');
                        player.knight_rang = temp.substr(0,pos);
                        player.knight_name = temp.substr(pos+1);
                    } else if(col.hasClass('memberLevel')) {
                        player.knight_level = parseInt(col.get('text'));
                    } else if(col.hasClass('memberSilver')) {
                        player.silver_spend = parseInt(col.get('text').replace(/\./g,''));
                    } else if(col.hasClass('memberRubies')) {
                        player.rubies_spend = parseInt(col.get('text').replace(/\./g,''));
                    } else if(col.hasClass('memberActivity')) {
                        var div = col.getFirst();
                        var temp = div.retrieve('tip:title');
                        var pos = temp.indexOf(': ');
                        player.last_activity = temp.substr(pos+2);
                    }
                },this);
                account.setPlayer(player);
                ret.members.push(player.knight_id);
            },this);

            ret.submodule = 'parseMembers';
            return(ret);
        } // parseMembers

        parseBattle = function(status) {
            var rounds = document.id('mainContent').getElement('.battlerounds');
            console.log(rounds.get('text').trim());
            status.battle_round = rounds.get('text').trim().split(' ')[1];
        }

        parsePrepare = function(status) {
            status.battle_round = 0;
        } // parsePrepare

        switch(path) {
            case 'members':
                ret = parseMembers();
                break;
            case 'fight':
            case 'battle':
                parseBattle(status);
                break;
            case 'prepare':
                parsePrepare(status);
                break;
            case 'upgrades':
            default:
                break;
        }
        status.submodule = path;

        return(ret);
    } // parseClan

    /*
     * parses the document of location baseUrl()+"manor"
    */
    parseManor = function(paths) {
        var path = paths.shift();
        var ret = {};

        parseManor = function() {
            var ret = {};

            for(i=1;i<7;++i) {
                var item = document.id('manorItem'+i);
                if(item.hasClass('manorItemActive')) {
                    var days = document.id('manorDays'+i);
                    ret['Item'+i] = parseInt(days.get('text'));
                }
            }

            ret.submodule = 'parseManor';
            return(ret);
        } // parseManor

        switch(path) {
            default:
                ret = parseManor();
                break;
        }

        return(ret);
    } // parseManor

    /*
     * parses the document of location baseUrl()+"mail"
    */
    parseMail = function(paths) {
        var path = paths.shift();
        var ret = {};

        parseReports = function() {
            var ret = {};

            return(ret);
        } // parseReports

        switch(path) {
            case 'reports':
                ret = parseReports();
                break;
            default:
                break;
        }

        return(ret);
    } // parseMail

    /*
     * parses the document of location baseUrl()+"highscore"
    */
    parseHighscore = function(paths) {
        var path = paths.shift();
        var ret = {};

        parseHighscore = function() {
            var ret = {};
            ret.players = [];

            var hsTable = document.id('highscoreTable');
            var hsBody = hsTable.getElement('tbody');
            var hsRows = hsBody.getElements('tr');
            hsRows.each(function(row,key){
                if(!row.hasClass('userSeperator')) {
                    var hsCols = row.getElements('td');
                    var player = {};
                    hsCols.each(function(col){
                        if(col.hasClass('highscore01')) {
                        } else if(col.hasClass('highscore02')) {
                            if(col.getFirst().hasClass('iconKarmaEvil')) player.course = 'evil';
                            else player.knight_course = 'good';
                        } else if(col.hasClass('highscore03')) {
                            var links = col.getElements('a');
                            player.knight_id = parseInt(links[0].pathname.split('/')[3]);
                            var temp = links[0].get('text');
                            var pos = temp.indexOf(' ');
                            player.knight_rang = temp.substr(0,pos);
                            player.knight_name = temp.substr(pos+1);
                            if(links.length === 2) {
                                player.clan_id = parseInt(links[1].pathname.split('/')[3]);
                                temp = links[1].get('text');
                                player.clan_tag = temp.substr(1,temp.length-2);
                            }
                        } else if(col.hasClass('highscore04')) {
                            player.knight_level = parseInt(col.get('text'));
                        } else if(col.hasClass('highscore05')) { // loot
                            player.loot_won = parseInt(col.get('text').replace(/\./g,''));
                        } else if(col.hasClass('highscore06')) { // fights
                            player.fights = parseInt(col.get('text').replace(/\./g,''));
                        } else if(col.hasClass('highscore07')) { // fights_won
                            player.fights_won = parseInt(col.get('text').replace(/\./g,''));
                        } else if(col.hasClass('highscore08')) { // fights_lose
                            player.fights_lose = parseInt(col.get('text').replace(/\./g,''));
                        }
                    },this);
                    account.setPlayer(player);
                    ret.players.push(player.knight_id);
                }
            },this);

            ret.submodule = 'parseHighscore';
            return(ret);
        } // parseReports

        switch(path) {
            case 'order':
                break;
            default:
                ret = parseHighscore();
                break;
        }

        return(ret);
    } // parseHighscore

    /*
     * parses the document in case of mainContent.hasClass('fightResult')
    */
    parseFightResult = function(status) {
        var ret = [];
        status.fightResult = true;
        if(status.module !== 'duel') return;

        // @disable-check M307
        ret.push(new kmTextoutModule(
                    {
                        name:'Aggressor',
                        template: 'kmZonesResult',
                        selector: 'fightResults',
                        position: 'left',
                        knight_id: status.knight_id
                    }));
        // @disable-check M307
        ret.push(new kmTextoutModule(
                    {
                        name:'Defender',
                        template: 'kmZonesResult',
                        selector: 'fightResults',
                        position: 'right',
                        knight_id: status.defender
                    }));

        return(ret);
    } // parseFightResult

    // Here comes the work:
    this.data.status = {};

    this.data.player = parsePlayer(status);
    if(document.id(document.body).hasClass('nonPremium')) this.data.player.premium = false;
    else this.data.player.premium = true;
    account.setProfile(this.data.player);

    var path = paths.shift();
    switch(path) {
        case 'common':
            this.data.status = parseCommon(status, paths);
            break;
        case 'user':
            this.data.status = parseUser(paths);
            break;
        case 'world':
            this.data.status = parseWorld(paths);
            break;
        case 'groupmission':
            this.data.status = parseGroupmission(paths);
            break;
        case 'market':
            this.data.status = parseMarket(paths);
            break;
        case 'duel':
            this.data.status = parseDuel(status, paths);
            break;
        case 'joust':
            this.data.status = parseJoust(status, paths);
            break;
        case 'clan':
        case 'clanwar':
            this.data.status = parseClan(status, paths);
            break;
        case 'manor':
            this.data.status = parseManor(paths);
            break;
        case 'mail':
            this.data.status = parseMail(paths);
            break;
        case 'highscore':
            this.data.status = parseHighscore(paths);
            break;
        default:
            this.data.status.module = null;
            break;
    }

    this.data.status.module = path;
    status.module = path;
    if(mainContent.hasClass('fightResult') && status.module === 'duel') {
        var result = parseFightResult(status);
        //console.log(JSON.stringify(typeof(result))+', '+JSON.stringify(result));
        if(typeof(result) === 'object' || typeof(result) === 'array') {
            for(var i=0; i<result.length; ++i) {
                this.gameModules.push(result[i]);
                //console.log(result[i].module_name+': setup Module with ' + JSON.stringify(result[i]));
            }
        }
    }

    account.setProfile(this.data.status);
    //account.setStatus(this.data.status);

    //console.log('parseDocument :'+JSON.stringify(status));
    account.setStatus(status);

/*
        template: 'kmZonesResult',
        selector: 'fightResults',
        position: 'left',
        knight_id: 0
*/
/*
    // @disable-check M307
    var test = new kmTextoutModule(
                {
                    name:'Defender',
                    template: 'kmZonesResult',
                    selector: 'fightResults',
                    position: 'right',
                    knight_id: status.defender
                });

    test.check('Zones');
    this.gameModules.push(test);
*/
    //var zones = JSON.parse(account.player(status.knight_id,'hitZones'));
    //console.log(typeof(zones)+', '+zones[0]);

}; // kM.prototype.parseDocument

