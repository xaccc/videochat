package videochat

class MediaEvent {
    
    String id
    String uid
    String sessionId
    
    String mediaServiceId
    
    Date dateLive
    Date dateLeave
    
    int duration = 0

    static constraints = {
        uid(maxSize:50,blank:false,nullable:false)
        sessionId(maxSize:200,blank:false,nullable:false)
        mediaServiceId(maxSize:200,blank:false,nullable:false)
        
        dateLive(nullable:true)
        dateLeave(nullable:true)
        
        duration(min:0)
    }

	static mapping = {
        id generator: 'uuid'
        sort dateLive: "desc"
	}
    
    String toString() {"$uid($sessionId)"}
}
