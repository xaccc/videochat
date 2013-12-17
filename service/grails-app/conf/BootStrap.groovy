import videochat.MediaService;
import videochat.Online;

class BootStrap {

    def init = { servletContext ->
        // if(MediaService.count() == 0) {
            // new MediaService(
                // name: '测试流服务器',
                // scheme: 'rtmp',
                // domain: '183.203.16.207',
                // port: 8100,
                // path: 'videochat'
                // ).save(flush:true);
        // }
    }
    
    def destroy = {
    }
}
