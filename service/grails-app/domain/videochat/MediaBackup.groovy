package videochat

class MediaBackup {
    
    String id

    String uid
    
    String domain
    String sessionId
    String flvid
    boolean first = false
    Date backupDate
    
    Date dateCreated
    Date lastUpdated
    
    static constraints = {
        uid(maxSize:50,blank:false,nullable:false)
        
        domain(maxSize:200,blank:false,nullable:false)
        sessionId(maxSize:200,blank:false,nullable:false)
        flvid(maxSize:200,blank:false,nullable:false)
        first(nullable:false)
        backupDate(nullable:false)
    }

	static mapping = {
        id generator: 'uuid'
	}
    
    String toString() {"$uid($flvid)"}
}
