var BOSH_SERVICE = '/http-bind/'
var connection = null;

function Chat() {
}

function message(msg) 
{
    $('#message').append('<div></div>').append(document.createTextNode(msg));
}

function onConnect(status)
{
    if (status == Strophe.Status.CONNECTING) {
        console.log('connecting.');
    } else if (status == Strophe.Status.CONNFAIL) {
        console.log('failed to connect.');
        $('#connect').get(0).value = 'connect';
    } else if (status == Strophe.Status.DISCONNECTING) {
        console.log('disconnecting.');
    } else if (status == Strophe.Status.DISCONNECTED) {
        $('[name=msessage]').hide();
        console.log('disconnected.');
        $('#connect').get(0).value = '登录';
        connection.connect(connection.jid, $('#pass').get(0).value, onConnect);
    } else if (status == Strophe.Status.CONNECTED) {
        $('#connect').get(0).value = '注销';
        console.log('connected.');
        console.log('JID: ' + connection.jid);
        console.log('SID: ' + connection.sid);
        console.log('RID: ' + connection.rid);

        connection.addHandler(onMessage, null, 'message', null, null,  null); 
        connection.send($pres().tree());
        
        $('[name=msessage]').show();

        // connect room
        connection.muc.join(
            'xx@vv.183.203.16.207', 
            connection.jid.split('@')[0], 
            onRoomMessage, onPresence, onRoster);
    }
}

function onMessage(msg) {
    var to = msg.getAttribute('to');
    var from = msg.getAttribute('from');
    var type = msg.getAttribute('type');
    var elems = msg.getElementsByTagName('body');

    if (type == "chat" && elems.length > 0) {
        var body = elems[0];

        message('[私聊]' + from.split('@')[0] + ': ' +
            Strophe.getText(body));
            
        console.log('get message from ' + from);
    }

    // we must return true to keep the handler alive.
    // returning false would remove it after it finishes.
    return true;
}

function onRoomMessage(stanza, room) {
    var from = stanza.getAttribute('from').split('/')[1];
    var type = stanza.getAttribute('type');
    var elems = stanza.getElementsByTagName('body');
    
    message((from ? from : '[消息]') + ': ' + 
            Strophe.getText(elems[0]));

    return true;
}

function onPresence(stanza, room) {
    console.log('onPresence stanza: ' + stanza);
    console.log('onPresence room  : ' + room);
    
    return true;
}

function onRoster(roster, room) {
    console.log('onPresence roster: ' + roster);
    console.log('onPresence room  : ' + room);
    
    return true;
}

$(document).ready(function () {
    $('[name=msessage]').hide();

    connection = new Strophe.Connection(BOSH_SERVICE);

    // Uncomment the following lines to spy on the wire traffic.
    connection.rawInput = function (data) { console.log('Strophe.RECV: ' + data); };
    connection.rawOutput = function (data) { console.log('Strophe.SEND: ' + data); };

    // Uncomment the following line to see all the debug output.
    Strophe.log = function (level, msg) { console.log('Strophe.LOG: ' + msg); };


    $('#connect').bind('click', function () {
        var button = $('#connect').get(0);
        if (button.value == '登录') {
            button.value = '注销';
            
            var jidField = $('#jid').get(0);
            var passField = $('#pass').get(0);

            connection.connect(jidField.value == '' ? '183.203.16.207' : jidField.value + '@183.203.16.207',
                               jidField.value == '' ? null : passField.value,
                               onConnect);
        } else {
            button.value = '登录';
            connection.disconnect();
        }
    });
    $('#connectAnonymous').bind('click', function () {
        $('#jid').get(0).value = '';
        $('#pass').get(0).value = '';
        $('#connect').trigger('click');
        $(this).hide();
    });
    $('#pass').keypress(function(event){
        if (event.which == 13) {
            $('#connect').trigger('click');
        }
    });
    
    
    $('#send').bind('click', function () {
        var button = $('#send').get(0);
        var to = $('#to').get(0).value;
        var text = $('#msg').get(0).value;
        
        var reply = $msg({to: to, from: connection.jid, type: 'chat'})
                        .c("body").t(text);
        connection.send(reply.tree());
        $('#msg').get(0).value = '';

        message('我 to ' + to.split('@')[0] + ': ' + text);
    });
    $('#msg').keypress(function(event){
        if (event.which == 13) {
            $('#send').trigger('click');
        }
    });
});
