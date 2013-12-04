package videochat

class Online {
    
    String id

    String uid
    
    String mediaServiceId
    String sessionId
    
    Date dateCreated
    Date lastUpdated
    
    static constraints = {
        uid(maxSize:50,blank:false,nullable:false,unique:true)
        
        mediaServiceId(maxSize:200,blank:false,nullable:false)
        sessionId(maxSize:200,blank:false,nullable:false)
    }

	static mapping = {
        id generator: 'uuid'
	}
    
    String toString() {"$uid($sessionId)"}
}
