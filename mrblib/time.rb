class Time
  def hwclock(timezone = nil)
    if timezone
      if timezone.include? "+"
        timezone = timezone.sub("+", "-")
      elsif timezone.include? "-"
        timezone = timezone.sub("-", "+")
      end
    end
    Platform::System.hwclock(self.year, self.month, self.day, self.hour, self.min, self.sec, timezone)
  end
end

