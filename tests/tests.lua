for _, cfg in ipairs( os.matchfiles("configs/*.lua") ) do
    include(cfg)
end

