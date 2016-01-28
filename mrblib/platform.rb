class Platform
  # Will be called at RunTime boot
  def self.setup
    Screen.setup(26, 12)
    begin
      require 'cloudwalk_handshake'
      CloudwalkHandshake.configure!
    rescue LoadError
    rescue NameError
    end
    # TODO Implement
  end
end

