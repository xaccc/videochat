package videochat

class MediaService {
    
    String id

    String name
    
    String scheme
    String domain
    int    port
    String path
    
    Date dateCreated
    Date lastUpdated
    
    static constraints = {
        name(maxSize:50,blank:false,nullable:false)
        
        scheme(maxSize:50,blank:false,nullable:false)
        domain(maxSize:255,blank:false,nullable:false)
        port(min:1,max:65535,nullable:false)
        path(maxSize:255,blank:false,nullable:false)
    }

	static mapping = {
        id generator: 'uuid'
	}
    
    String toString() {"$name(${scheme}:://${domain}:${port}${path})"}
}
